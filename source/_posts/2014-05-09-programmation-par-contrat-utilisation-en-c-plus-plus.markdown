---
layout: post
title: "Programmation par contrat -- utilisation en C++"
date: 2014-05-09 12:07:42 +0200
comments: true
categories: C++
published: true
footer: true
---

Dans ce second billet sur la _Programmation par Contrat_, je vais vous présenter
quelques techniques d'application de la PpC au C++.

## <a id="Documentation"></a>Documentation

Comme je l'avais signalé dans le précédent billet, la première chose que l'on
peut faire à partir des contrats, c'est de les documenter clairement.
Il s'agit probablement d'une des choses les plus importantes à documenter dans
un code source. Et malheureusement trop souvent c'est négligé.

L'outil [Doxygen](http://doxygen.org) met à notre disposition les tags `@pre`,
`@post`, et `@invariant` pour documenter nos contrats. Je ne peux que
vous conseiller d'en user et d'en abuser.


## Que faire des contrats ?

À partir d'un contrat bien établi, nous avons techniquement plusieurs choix en
C++. 


### Option 1 : on ne fait rien

Il est tout d'abord possible de totalement ignorer les ruptures de
contrats et de ne jamais rien vérifier.

Quand une erreur de programmation survient, quand on est chanceux, on détecte
le problème au plus proche de l'erreur. Malheureusement, en C et en C++, les
problèmes tendent à survenir bien plus tard.

Cette mauvaise pratique consistant à ignorer les contrats (ou à ne pas s'en
préoccuper) est assez répandue. Je ne cache pas que l'un des objectifs de ces
billets est de combattre cette habitude.

### Option 2 : on lance des exceptions dans la tradition de la Programmation Défensive

À l'opposé, on peut prendre la voie de la _Programmation Défensive_ et vérifier
chaque rupture potentielle de contrat pour lever une exception. Au delà des
problèmes de conceptions et de déresponsabilisation évoqués dans le billet
précédent, il y a un soucis technique.

En effet, en temps normal avec une exception en C++, on ne peut rien avoir de
mieux que des informations sur le lieu de la détection. Et encore faut-il
disposer de classes _exception_ qui stockent une telle information ; ce n'est
par exemple par le cas des `std::logic_error` qui sont levées depuis des
fonctions comme `std::vector<>::at()`.

Par _rien de mieux que le lieu de la détection_, il faut comprendre que l'on ne
disposera d'aucune autre information de contexte. En effet, une exception
remonte jusqu'à un `catch` compatible ; or à l'endroit du `catch`, on ne peut
plus avoir accès à l'état (de toutes les variables, dans tous les threads, ...)
au moment de la détection du problème.

En vérité, il y existe deux moyens peu ergonimiques d'y avoir accès. Le prémier
consiste à mettre des points d'arrêt sur les levers ou les constructions
d'exception, et à exécuter le programme depuis un débuggueur. Le second
consiste à supprimer du code source tous les `catchs` qui sont compatibles avec
l'erreur de logique.

Aucune des deux options n'est véritablement envisageable pour des tests
automatisés. Elles le sont en revanche pour investiguer.

À noter aussi qu'avec cette approche, on paie tout le temps un coût de
vérification des contrats, que cela soit en phase de tests comme en phase de
production.  Et ce, même pour des morceaux de code où il est certain qu'il n'y
a pas d'erreur de programmation.  
Par exemple, `sqrt(1-sin(x))` ne devrait poser aucun soucis. Une fonction sinus
renvoie en théorie un nombre entre -1 et 1, ce qui constitue une post-condition
toute indiquée. De fait par construction, `1-sin(x)` est positif, et donc
compatible avec le contrat de `sqrt`.

### Option 3 : on formalise nos suppositions à l'aide d'assertions

Le C, et par extension le C++, nous offrent un outil tout indiqué pour traquer
les erreurs de programmation : les assertions.

En effet, compilé sans la directive de précompilation `NDEBUG`, une assertion
va arréter un programme et créer un fichier _core_. Il est ensuite possible
d'ouvrir le fichier _core_ depuis le debuggueur pour pouvoir explorer l'état du
programme au moment de la détection de l'erreur.

#### Exemple d'exploitation des assertions

Sans faire un cours sur gdb, regardons ce qu'il se passe sur ce petit programme :

```c++
// test-assert.cpp
#include <iostream>
#include <cmath>
#include <cassert>

namespace my {
    double sqrt(double n) {
        assert(n >=0);
        return std::sqrt(n);
    }

    double sin(double n) {
        const double r = std::sin(n);
        assert(r <= 1 && r >= -1);
        return r;
    }
} // my namespace


int main ()
{
    std::cout << my::sqrt(my::sin(0)-1) << std::endl;
}
// Vim: let $CXXFLAGS='-g'

```

Exécuté en console, on verra juste :
```
$ ./test-assert
assertion "n >=0" failed: file "test-assert.cpp", line 7, function: double my::sqrt(double)
Aborted (core dumped)
```

On dispose de suite de l'indication où l'erreur a été détectée.
Mais investigons plus en avant. Si on lance `gdb ./test-assert core.pid42`
(cela peut nécessiter de demander à `ulimit` d'autoriser les _coredumps_ sur
votre plateforme, faites un `ulimit -c unlimited` pour cela), ou `gdb
./test-assert` puis `run` pour une investigation pseudo-interactive, on observe
ceci :

```
$ gdb test-assert
GNU gdb (GDB) 7.6.50.20130728-cvs (cygwin-special)
Copyright (C) 2013 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "i686-pc-cygwin".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
<http://www.gnu.org/software/gdb/documentation/>.
For help, type "help".
Type "apropos word" to search for commands related to "word".
..
Reading symbols from /cygdrive/c/Dev/blog/source/_posts/test-assert...done.
(gdb) run
Starting program: /cygdrive/c/Dev/blog/source/_posts/test-assert
[New Thread 5264.0xe2c]
[New Thread 5264.0x6fc]
assertion "n >=0" failed: file "test-assert.cpp", line 7, function: double my::sqrt(double)

Program received signal SIGABRT, Aborted.
0x0022da18 in ?? ()
```

La pile d'appels (_back trace_) contient :
```
(gdb) bt
#0  0x0022da18 in ?? ()
#1  0x7c802542 in WaitForSingleObject () from /cygdrive/c/WINDOWS/system32/kernel32.dll
#2  0x610da840 in sig_send(_pinfo*, siginfo_t&, _cygtls*) () from /usr/bin/cygwin1.dll
#3  0x610d7c7c in _pinfo::kill(siginfo_t&) () from /usr/bin/cygwin1.dll
#4  0x610d8146 in kill0(int, siginfo_t&) () from /usr/bin/cygwin1.dll
#5  0x610d8312 in raise () from /usr/bin/cygwin1.dll
#6  0x610d85b3 in abort () from /usr/bin/cygwin1.dll
#7  0x61001aed in __assert_func () from /usr/bin/cygwin1.dll
#8  0x004011d3 in my::sqrt (n=-1) at test-assert.cpp:7
#9  0x0040125a in main () at test-assert.cpp:21
```

Avec un `up 8` pour se positionner au niveau où l'assertion est fausse, on peut
regarder le code source local avec un `list`, ou de façon plus intéressante,
demander ce que vaut ce fameux `n`.

```
(gdb) up 8
#8  0x004011d3 in my::sqrt (n=-1) at test-assert.cpp:7
7               assert(n >=0);
(gdb) p n
$1 = -1
``` 

On voit que `my::sqrt` a été appelée avec `-1` comme paramètre. Avec un `up` on
peut investiguer le contexte de la fonction appelante -- avec `down` on
progresse dans l'autre sens. Ici, la raison de l'erreur est triviale. Dans un
programme plus complexe, on aurait pu imaginer que `sin` était appelée avec une
donnée non constante, et on aurait peut-être passé un peu plus de temps à
comprendre que la fonction fautive n'était pas `sin` mais ce que l'on faisait
avec son résultat.


NB: l'équivalent existe pour d'autres environnements comme VC++.

#### Un outil pour les phases de développement et de tests ...

Les assertions ne sont vérifiées que si `NDEBUG` n'est pas défini au moment de
la précompilation. Généralement, sa définition accompagne les mode _Release_ de
VC++ et de CMake. Ce qui veut dire qu'en mode _Release_ aucune assertion n'est
vérifiée, soit qu'en production, les assertions sont normalement ignorées. Le
corrolaire de tout cela est que les assertions sont un outil de vérification de
la logique applicative qui n'est utilisé qu'en phases de développement et de
tests.

détection en phase compilation > TU > Test intégration/validation > production
(A rédiger...)

#### ... voire de production

Prog def si vraiment on veut -> on définit comme il nous plait les macros.
(A rédiger...)

#### Techniques connexes
paramètre -> message `assert(condition && "explications");` ou `assert(!"explications");`
(A rédiger...)


#### Exploitation des assertions par les outils d'analyse statique de code

À mon grand regret, les outils d'analyse statique de code comme [clang
analyzer](http://clang-analyzer.llvm.org/) ne semblent pas exploiter les
assertions pour détecter statiquement des erreurs de logique, mais pour inhiber
l'analyse de certains chemins d'éxécutions.

Ainsi, dans l'exemple de `test-assert.cpp` que j'ai donné plus haut, les outils
d'analyse statique de code ne feront pas le rapprochement entre la
post-condition de `my::sin` et la pré-conditon de `my::sqrt`, mais feront
plutôt comme si les assertions étaient toujours vraies, c'est à dire comme si
le code n'appelait jamais `my::sin` avec un nombre négatif.

NB: Je généralise à partir de mon test avec clang analyzer. Peut-être que
d'autres outils savent tirer parti des contrats déclarés à l'aide d'assertions,
ou peut-être le sauront-ils demain.


## Snippets de code

### Pré- et post-conditions de fonctions

#### Pré- et post-conditions de fonctions membres, à la Non-Virtual Interface Pattern (NVI)

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

#### Pré- et post-conditions de fonctions, à la Imperfect C++

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

### Invariants de classes

#### Petit snippet de vérification simplifiée en l'absence d'héritage

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

### propal C++14/17:
(A rédiger...)

http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3753.pdf


## Invariants statiques

(A rédiger...)

- références
- boost.unit
- objet pertinent
- les divers types de pointeurs (cf. prochain billet)
- init vs constructeur

## Remerciements

Un grand merci à tous mes relecteurs,
Guilhem Bonnefille,
David Côme,
Philippe Lacour,
Cédric Poncet-Montange
