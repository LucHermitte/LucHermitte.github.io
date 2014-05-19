---
layout: post
title: "Programmation par Contrat 3/3 -- snippets pour le C++"
date: 2014-05-13 11:33:15 +0200
comments: true
categories: C++
published: true
footer: true
---

Dans ce dernier billet sur la _Programmation par Contrat_, je vais vous présenter
quelques techniques d'application de la PpC au C++.

## Pré- et post-conditions de fonctions

### Pré- et post-conditions de fonctions membres, à la Non-Virtual Interface Pattern (NVI)

Le pattern NVI est un _Design Pattern_ qui ressemble au DP _Template Method_ mais qui
n'est pas le _Template Method_. Le principe du pattern est le suivant :
l'interface publique est non virtuelle, et elle fait appel à des comportements
spécialisés qui sont eux privés et virtuels (généralement virtuels purs).

Ce pattern a deux objectifs avoués. Le premier est de découpler les interfaces
pour les _utilisateurs_ du pattern. Le code client doit passer par l'interface
publique qui est non virtuelle, tandis que le code qui spécialise doit
s'intéresser à l'interface privée et virtuelle.

Le second objectif, est de créer des super-interfaces qui baignent dans la
PpC. Les interfaces classiques à la Java/C#/COM/CORBA/... ne permettent pas
d'associer nativement des contrats à leurs méthodes. Avec le pattern NVI on
peut, avec un soupçon d'huile de coude, rajouter des contrats aux fonctions
membres.

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
    virtual double do_compute(int x) const =0;
};

class Impl : Contract1, Contract2
{
private:
    virtual double do_compute(int x) const { ... }
    // + spécialisations des fonctions de Contract2
};
```

Pour ce qui est de gérer en plus les invariants de tous les contrats, et des
classes finales. Je partirai avec un héritage virtuel depuis une classe de base
virtuelle `WithInvariants` dont la fonction de vérification serait spécialisée
par tous les intermédiaires. Et dont les intermédiaires appelleraient toutes
versions mères pour n'oublier personne. 

```c++
struct WithInvariants : boost::noncopyable {
    void check_invariants() const {
ifndef NDEBUG
        do_check_invariants();
endif
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
    virtual void do_check_invariants() const {
        assert(invariant C1 ...);
    }
    ....
}

struct Impl : Contract1, Contract2
{
    ....
protected:
    virtual void do_check_invariants() const {
        Contract1::do_check_invariants();
        Contract2::do_check_invariants();
        assert(invariant rajoutés par Impl ...);
    }
};
```

(Pour l'instant, je n'ai pas de meilleure idée)

### Pré- et post-conditions de fonctions, à la Imperfect C++

Matthew Wilson consacre le premier chapitre de son [_Imperfect C++_](#IPCpp) à
la PpC. Je ne peux que vous en conseiller la lecture.

Il y présente au §I.1.3 la technique suivante :

```c++
double my::sqrt(double n)
if defined(MYLIB_DBC_ACTIVATED)
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
endif
{
    return std::sqrt(n);
}
```

## Invariants de classes

### Petit snippet de vérification simplifiée en l'absence d'héritage

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

NB: je vois à la relecture d'_Imperfect C++_ que c'est très proche de ce que
suggérait Matthew Wilson. Au détail qu'il passe par une fonction `is_valid`
renvoyant un booléen et que l'`InvariantChecker` s'occupe de vérifier
l'assertion si `MYLIB_DBC_ACTIVATED` est bien défini -- il découple la
vérification des contrats de la macro `NDEBUG` qui est plus liée au mode de
compilation (_Débug_ VS _Release_).  
Pour ma part, je préfère avoir une assertion différente pour chaque invariant
plutôt qu'un seul `assert(is_valid());`. Cela permet de savoir plus précisément
quel contrat est violé.

### Critiques envisageables avec cette approche
On peut s'attendre qu'en cas d'exception dans une fonction membre (ou amie)
d'un objet, l'invariant ne soit plus respecté. Dans ce cas là, l'approche
proposée juste au dessus va poser d'énormes problèmes.

Toutefois cela voudrait dire que l'exception ne laisse plus l'objet dans un
état cohérent, et que nous n'avons pas la
[_garantie basique_](http://en.wikipedia.org/wiki/Exception_safety).

Dans le même genre d'idée, nous nous retrouverions dans la même situation que
si on utilisait des constructeurs qui ne garantissent pas l'invariant de leurs
classes, et qui sont utilisés conjointement avec des fonctions `init()`. En
effet, si l'invariant ne peut plus être assuré statiquement par programmation,
il est nécessaire de l'assurer dynamiquement en vérifiant en debut de chaque
fonction membre (/amie) si l'objet est bien valide.

C'est pour tout ces raisons que je ne suis pas d'accord avec cette critique.
(TODO: à reformuler)
