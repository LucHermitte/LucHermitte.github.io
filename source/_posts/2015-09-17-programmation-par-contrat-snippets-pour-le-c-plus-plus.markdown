---
layout: post
title: "Programmation par Contrat 3/3"
subtitle: "Snippets pour le C++"
date: 2015-09-17 19:20:15 +0200
comments: true
categories: C++
published: true
footer: true
toc: true
language: fr
---

Dans ce dernier billet sur la _Programmation par Contrat_, je vais vous présenter
quelques techniques d'application de la PpC au C++. Ce billet décrivant des
techniques sera plus décousu que les précédents qui avaient un fil conducteur.

_(Désolé, j'ai mis du temps à mûrir certains de ses paragraphes)_

## I- Pré- et post-conditions de fonctions.

### I.1- Pré- et post-conditions de fonctions membres, à la Non-Virtual Interface Pattern (NVI).

Le pattern NVI est un _Design Pattern_ qui ressemble au DP _Template Method_
mais qui n'est pas le
[_Template Method_](http://fr.wikipedia.org/wiki/Patron_de_m%C3%A9thode). Le
principe du pattern est le suivant : l'interface publique est non virtuelle, et
elle fait appel à des comportements spécialisés qui sont eux privés et virtuels
(généralement virtuels purs).

Ce pattern a deux objectifs avoués. Le premier est de découpler les interfaces
pour les _utilisateurs_ du pattern. Le code client doit passer par l'interface
publique qui est non virtuelle, tandis que le code qui spécialise doit
s'intéresser à l'interface privée et virtuelle.

Le second objectif, est de créer des super-interfaces qui baignent dans la
PpC. Les interfaces classiques à la Java (up to v7)/C#/COM/CORBA/... ne
permettent pas d'associer nativement des contrats à leurs méthodes. Avec le
pattern NVI on peut, avec un soupçon d'huile de coude, rajouter des contrats
aux fonctions membres.

Les fonctions publiques et non virtuelles se voient définies `inline`s, elles
vérifient en premier lieu pré-conditions et invariants, elles exécutent ensuite
le code spécialisé, et elles finissent par vérifier post-conditions et
invariants.  
Soit:

```c++
/** Interface/contrat C1.
 */
struct Contract1 : boost::noncopyable
{
    virtual ~Contract1(){};

    /** @pre <tt> x > 42</tt>, vérifié par assertion.
     */
    double compute(double x) const {
        assert(x > 42 && "echec de précondition sur contrat1");
        return do_compute(x);
    }
private:
    virtual double do_compute(int x) const = 0;
};

class Impl : Contract1, Contract2
{
private:
    virtual double do_compute(int x) const override { ... }
    // + spécialisations des fonctions de Contract2
};
```

Je reviendrai [plus loin](#NVI_Invariants) sur une piste pour supporter des
invariants dans un cadre de NVI.

### I.2- Pré- et post-conditions de fonctions, à la Imperfect C++.

Matthew Wilson consacre le premier chapitre de son [_Imperfect C++_](#IPCpp) à
la PpC. Je ne peux que vous en conseiller la lecture.

Il y présente au §I.1.3 la technique suivante :

```c++
double my::sqrt(double n)
#if defined(MYLIB_DBC_ACTIVATED)
{
    // Check pre-conditions
    assert(n>=0 && "sqrt can't process negative numbers");
    // Do the work
    const double res = my::sqrt_unchecked(n);
    // Check post-conditions
    assert(std::abs(res*res - n)<epsilon && "Invalid sqrt result");
    return res;
}
double my::sqrt_unchecked(double n)
#endif
{
    return std::sqrt(n);
}
```

### I.3- Pré- et post-conditions de fonctions ... ` constexpr` C++11.
Les fonctions `constexpr` à la C++11 doivent renvoyer une valeur et ne rien
faire d'autre. De plus le contrat doit pouvoir être vérifié en mode _appelé
depuis une expession constante_ comme en mode _appelé depuis une expression
variable_. De fait, cela nécessite quelques astuces pour pouvoir spécifier des
contrats dans de telles fonctions.

Pour de plus amples détails, je vous renvoie à
l'[article](http://ericniebler.com/2014/09/27/assert-and-constexpr-in-cxx11/)
fort complet d'Eric Niebler sur le sujet. Andrzej présente la même technique
dans son article
[Compile Time Computations](http://akrzemi1.wordpress.com/2011/05/06/compile-time-computations/).

En résumé, on peut procéder de la sorte. Avec ceci:
```c++
/** Helper struct for DbC programming in C++11 constexpr functions.
 * Copyright 2014 Eric Niebler,
 * http://ericniebler.com/2014/09/27/assert-and-constexpr-in-cxx11/
 */
struct assert_failure
{
    template<typename Fun>
    explicit assert_failure(Fun fun)
    {
        fun();
        // For good measure:
        std::quick_exit(EXIT_FAILURE);
    }
};
```
On peut ainsi exprimer des fonctions `constexpr` en C++11 :
```c++
/**
 * Internal constexpr function that computes \f$n!\f$ with a tail-recursion.
 * @param[in] n
 * @param[in] r  pre-computed result
 * @pre n shall not induce an integer overflow
 * @post the result won't be null
 * @author Luc Hermitte
 */
constexpr unsigned int fact_impl(unsigned int n, unsigned int r) {
    return
        n <= 1                                          ? r
#ifndef NDEBUG
        : std::numeric_limits<decltype(n)>::max()/n < r ? throw assert_failure( []{assert(!"int overflow");})
#endif
        :                                                 fact_impl(n-1, n*r)
        ;
}
constexpr unsigned int fact(unsigned int n) {
    return fact_impl(n, 1);
}

int main() {
    const unsigned int n10 = fact(10);
    const unsigned int n50 = fact(50);
}
```

Malheureusement la rupture de contrat ne sera pas détectée lors de la
compilation, mais à l'exécution où l'on pourra constater à minima où l'appel de
plus haut niveau s'est produit (bien que l'on risque de ne pas pouvoir observer
l'état des variables _optimized out_ dans le débuggueur).

Notez que pour exprimer une post-condition sans multiplier les appels, j'ai
écrit la fonction (qui aurait été récursive dans tous les cas) en
[fonction récursive terminale](http://fr.wikipedia.org/wiki/Récursion_terminale).
De là, il a été facile d'insérer une assertion -- et de plus, le compilateur
pourra optimiser la fonction en _Release_ sur les appels dynamiques.

Pour information, une autre écriture qui exploite l'opérateur virgule est
possible, mais elle ne compile pas avec les versions de GCC que j'ai eu entre
les mains (i.e. jusqu'à la version 4.9, GCC n'est pas d'accord).

```c++
/**
 * Internal constexpr function that computes \f$n!\f$ with a tail-recursion.
 * @param[in] n
 * @param[in] r  pre-computed result
 * @pre n shall not induce an integer overflow
 * @post the result won't be null
 * @warning This version does not compile with GCC up-to v4.9.
 * @author Luc Hermitte
 */
constexpr unsigned int fact_impl(unsigned int n, unsigned int r) {
    return n >= 1
        // ? (assert(std::numeric_limits<decltype(n)>::max()/n >= r), fact_impl(n-1, n*r))
        ? fact_impl((assert(std::numeric_limits<decltype(n)>::max()/n >= r), n-1), n*r)
        : (assert(r>0), r);
}
```


N.B.: Dans le cas des `constexpr` du C++14, il me faudrait vérifier si `assert()` est
directement utilisable. A priori, cela sera le cas.

## II- Invariants de classes.

### II.1- Petit snippet de vérification simplifiée en l'absence d'héritage.

Sur un petit [exercice d'écriture de classe fraction](http://ideone.com/DOCWOy),
j'avais pondu une classe utilitaire dont le but était de simplifier la
vérification des invariants. Il suffit de déclarer un objet de ce type en tout
début des fonctions membres (et des fonctions amies) exposées aux clients.
Ainsi les invariants sont automatiquement vérifiés en début, et en fin de
fonction lors de la destruction de l'objet `InvariantChecker`.

```c++
/** Helper class to check invariants.
 * @tparam CheckedClass shall define a \c check_invariants fonction where
   invariants checking is done.
 */
template <typename CheckedClass>
struct InvariantChecker {
    InvariantChecker(CheckedClass const& cc_) : m_cc(cc_)
    { m_cc.check_invariants(); }
    ~InvariantChecker()
    { m_cc.check_invariants(); }
private:
    CheckedClass const& m_cc;
};

/** rational class.
 * @invariant <tt>denominator() > 0</tt>
 * @invariant visible objects are normalized.
 */
struct Rational {
    ....
    // Une fonction publique qui doit vérifier l'invariant
    Rational & operator+=(Rational const& rhs) {
        InvariantChecker<Rational> check(this);
        ... le code de l'addition ...
        return *this;
    }

private:
    // La fonction interne de vérification
    void check_invariants() const {
        assert(denominator() && "Denominator can't be null");
        assert(denominator()>0 && "Denominator can't be negative");
        assert(pgcd(std::abs(numerator()), denominator()) == 1 && "The rational shall be normalized");
    }
    // Et on donne accès à la classe InvariantChecker<>
    friend class InvariantChecker<rational>;

    ... les membres ...
}
```

N.B.: je vois à la relecture d'_Imperfect C++_ que c'est très proche de ce que
suggérait Matthew Wilson. Au détail qu'il passe par une fonction `is_valid`
renvoyant un booléen et que l'`InvariantChecker` s'occupe de vérifier
l'assertion si `MYLIB_DBC_ACTIVATED` est bien défini -- il découple la
vérification des contrats de la macro `NDEBUG` qui est plus liée au mode de
compilation (_Débug_ VS _Release_).  
Pour ma part, je préfère avoir une assertion différente pour chaque invariant
plutôt qu'un seul `assert(is_valid());`. Cela permet de savoir plus précisément
quel contrat est violé.

### <a id="NVI_Invariants"></a>II.2- Invariants et NVI.

Pour ce qui est de gérer les invariants de plusieurs contrats, et des classes
finales. Je partirai sur un héritage virtuel depuis une classe de base
virtuelle `WithInvariants` dont la fonction de vérification serait spécialisée
par tous les intermédiaires. Et dont les intermédiaires appelleraient toutes
les versions mères pour n'oublier personne.

```c++
struct WithInvariants : boost::noncopyable {
    void check_invariants() const {
#ifndef NDEBUG
        do_check_invariants();
#endif
    }
protected:
    ~WithInvariants() {}
    virtual void do_check_invariants() const {}
};

struct InvariantChecker {
    InvariantChecker(WithInvariants const& wi) : m_wi(wi)
    { m_wi.check_invariants(); }
    ~InvariantChecker()
    { m_wi.check_invariants(); }
private:
    WithInvariants const& m_wi;
};

struct Contract1 : boost::noncopyable, virtual WithInvariants
{
    ...
    double compute(double x) const {
        ...preconds...
        InvariantChecker(*this);
        return do_compute(x);
    }
protected:
    virtual void do_check_invariants() const override {
        assert(invariant C1 ...);
    }
    ....
}

struct Impl : Contract1, Contract2
{
    ....
protected:
    virtual void do_check_invariants() const override {
        Contract1::do_check_invariants();
        Contract2::do_check_invariants();
        assert(invariant rajoutés par Impl ...);
    }
};
```

(Alors certes, c'est tordu, mais pour l'instant, je n'ai pas de meilleure idée.)

### II.3- Critiques envisageables avec ces approches.
On peut s'attendre qu'en cas d'exception dans une fonction membre (ou amie)
d'un objet, l'invariant ne soit plus respecté.  
Dans ce cas là, les approches proposées juste au dessus vont poser d'énormes
problèmes.

Toutefois cela voudrait dire que l'exception ne laisse plus l'objet dans un
état cohérent, et que nous n'avons pas la
[_garantie basique_](http://en.wikipedia.org/wiki/Exception_safety).

Autre scénario dans le même ordre d'idée : imaginez que les flux aient pour
invariant `good()`, et qu'une extraction ratée invalide le flux.  Cette fois,
l'objet pourrait exister dans un état contraire à son invariant, ce qui ferait
claquer l'assertion associée

Dans le même genre d'idée, nous nous retrouverions dans la même situation que
si on utilisait des constructeurs qui ne garantissent pas l'invariant de leurs
classes, et qui sont utilisés conjointement avec des fonctions `init()`. En
effet, si l'invariant ne peut plus être assuré statiquement par programmation,
il est nécessaire de l'assurer dynamiquement en vérifiant en début de chaque
fonction membre (/amie) si l'objet est bien valide.

Effectivement il y a alors un problème. À mon avis, le problème n'est pas dans
le fait de formuler les invariants de notre objet et de s'assurer qu'ils soient
toujours vérifiés. Le problème est de permettre à l'objet de ne plus vérifier
ses invariants et qu'il faille le tester dynamiquement.

#### Les objets _cassés_

On retrouve le modèle des flux de données (fichiers, sockets, ...) qui peuvent
passer KO et qu'il faudra rétablir. Dans cette approche, plutôt que de se
débarrasser du flux pour en construire un tout beau tout neuf, on le maintient
(car après tout il est déjà initialisé) et on cherchera à le reconnecter.

Plus je réfléchis à la question et moins je suis friand de ces objets qui
peuvent être cassés.

Dans un monde idéal, j'aurai tendance à dire qu'il faudrait établir des zones
de codes qui ont des invariants de plus en plus précis -- les invariants étant
organisés de façon hiérarchique.

Dans la zone _descriptif de flux configuré_, il y aurait la zone _flux valide
et connecté_. Quand le flux n'est plus valide, on peut retourner à la zone
englobante de flux décrit. C'est d'ailleurs ce qu'on l'on fait d'une certaine
façon. Sauf que nous avons pris l'habitude (avec les abstractions de sockets et
de fichiers usuelles) de n'avoir qu'un seul objet pour contenir les deux
informations. Et de fait, quand on veut séparer les deux invariants à
l'exécution, on se retrouve avec des objets cassés...

La solution ? Ma foi, le SRP (_Single Responsability Principle_) me semble
l'apporter : _«un object, une responsabilité»_. On pourrait même dire :

> Deux invariants décorrélés (/non synchrones) => deux classes.

### II.4- Des exceptions dans les constructeurs.
Une technique bien connue pour prévenir la construction d'un objet dont on ne
peut pas garantir les invariants consiste à lever une exception depuis son
constructeur. En procédant de la sorte, soit un objet existe et il est dans un
état pertinent et utilisable, soit il n'a jamais existé et on n'a même pas
besoin de se poser la question de son utilisabilité.

Cela a l'air fantastique, n'est-ce pas ?

Mais ... n'est-ce pas de la programmation défensive ? En effet, ce n'est pas le
client de l'objet qui vérifie les conditions d'existence, mais l'objet.
Résultat, on ne dispose pas forcément du
[meilleur contexte]({%post_url 2014-05-24-programmation-par-contrat-un-peu-de-theorie%}#ProgDefCtx) pour
signaler le problème de _runtime_ qui bloque la création de l'objet.

Idéalement, je tendrais à dire que la vérification devrait être faite en amont,
et ainsi le constructeur aurait des pré-conditions _étroitement_ vérifiées.
Dans la pratique, je dois bien avouer que je tends, aujourd'hui, à laisser la
vérification au niveau des constructeurs au lieu d'exposer une fonction
statique de vérification des pré-conditions d'existence dans les cas les plus
complexes. Il faut dire que les exceptions ont tellement été bien vendues comme
: c'est le seul moyen d'avorter depuis un opérateur surchargé ou depuis un
constructeur, que j'ai jusqu'à lors totalement négligé mon instinct qui sentait
qu'il y avait un truc louche à vérifier les conditions de création depuis un
contexte restreint. À _élargir_ les contrats, on finit par perdre des
informations pour nos messages d'erreur.

## III- Et si la Programmation Défensive est de la partie ?

_Discl. : [L'utilisation de codes de retour va grandement complexifier l'application](http://isocpp.org/wiki/faq/exceptions#exceptions-avoid-spreading-out-error-logic),
qui en plus de devoir tester les codes de retour relatifs au métier (dont la
validation des entrées), devra propager des codes de retours relatifs aux
potentielles erreurs de programmation. Au final, cela va accroitre les chances
d'erreurs de programmation... chose antinomique avec les objectifs de la
technique. Donc un conseil, pour de la programmation défensive en C++, préférez
l'emploi d'exceptions -- et bien évidemment, n'oubliez pas le
[RAII]({%post_url 2012-04-04-le-c-plus-plus-moderne%}#C++Moderne), notre grand
ami._

Prérequis : dérivez de
[`std::runtime_error`](http://www.cpluscplus.com/reference/stdexcept/runtime_error/)
vos exceptions pour les cas exceptionnels pouvant se produire lors de l'exécution,
et de
[`std::logic_error`](http://www.cpluscplus.com/reference/stdexcept/logic_error/)
vos exceptions pour propager les erreurs de programmation.

Plusieurs cas de figures sont ensuite envisageables.

### III.1- Cas théorique idéal...

... lorsque COTS et bibliothèques tierces __ne__ dérivent __pas__ leurs
exceptions de `std::exception` __mais__ de `std::runtime_error` pour les cas
exceptionnels plausibles et de `std::logic_error` pour les erreurs de logique.

Aux points d'interfaces (communication via une API C, limites de threads en
C++03), ou dans le `main()`, il est possible de filtrer les erreurs de logiques
pour avoir des coredumps en _Debug_.

```c++
int main()
{
   try {
        leCodeQuipeutprovoquerDesExceptions();
        return EXIT_SUCCESS;
#ifdef NDEBUG
   } catch (std::logic_error const& e) {
      std::cerr << "Logic error: " << e.what() << "\n";
#endif
   } catch (std::runtime_error const& e) {
      std::cerr << "Error: " << e.what() << "\n";
   }
   return EXIT_FAILURE;
}
```

Il est à noter que ce cas théorique idéal se combine très mal avec les
techniques de
[dispatching](http://www.parashift.com/c++-faq/throw-without-an-object.html) et
de
[factorisation](http://isocpp.org/wiki/faq/exceptions#throw-without-an-object)
de gestion des erreurs. En effet, tout repose sur un `catch(...)`, or ce
dernier va modifier le contexte pour la génération d'un _core_ tandis que rien
ne sera redispatché vers une `std::logic_error`.

### III.2- Cas plausible...

... lorsque COTS et bibliothèques tierces dérivent _malheureusement_ leurs
exceptions de `std::exception` __au lieu__ de `std::runtime_error` pour les cas
exceptionnels plausibles et de `std::logic_error` pour les erreurs de logique.

Aux points d'interfaces (communication via une API C, limites de threads en
C++03), ou dans le `main()`, il est possible d'ignorer toutes les exceptions pour
avoir des coredumps en _Debug_ sur les exceptions dues à des erreurs de logiques et ...
sur les autres aussi.

```c++
int main()
{
#ifdef NDEBUG
   try {
#endif
       leCodeQuipeutprovoquerDesExceptions();
       return EXIT_SUCCESS;
#ifdef NDEBUG
   } catch (std::exception const& e) {
       std::cerr << "Error: " << e.what() << "\n";
   }
   return EXIT_FAILURE;
#endif
}
```

D'autres variations sont très certainement envisageables où l'on rattraperait
l'erreur de logique pour la relancer en _Debug_.
