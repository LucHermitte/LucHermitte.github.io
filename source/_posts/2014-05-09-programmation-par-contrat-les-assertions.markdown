---
layout: post
title: "Programmation par contrat 2/3 -- les assertions"
date: 2014-05-09 12:07:42 +0200
comments: true
categories: C++
published: true
footer: true
---

Dans ce second billet sur la _Programmation par Contrat_, nous allons voir que
faire des contrats une fois établis, et en particulier je vais vous présenter
l'outil par excellence pour détecter les erreurs de programmation : les
_assertions_.

## <a id="Documentation"></a>Documentation

Comme je l'avais signalé dans le précédent billet, la première chose que l'on
peut faire à partir des contrats, c'est de les documenter clairement.
Il s'agit probablement d'une des choses les plus importantes à documenter dans
un code source. Et malheureusement trop souvent c'est négligé.

L'outil [Doxygen](http://doxygen.org) met à notre disposition les tags `@pre`,
`@post`, et `@invariant` pour documenter nos contrats. Je ne peux que
vous conseiller d'en user et d'en abuser.


## Comment prendre en compte les contrats dans le code ?

À partir d'un contrat bien établi, nous avons techniquement plusieurs choix en
C++. 


### Option 1 : on ne fait rien

Il est tout d'abord possible de totalement ignorer les ruptures de
contrats et de ne jamais rien vérifier.

Quand une erreur de programmation survient, quand on est chanceux, on détecte
le problème au plus proche de l'erreur. Malheureusement, en C et en C++, les
problèmes tendent à survenir bien plus tard.

Cette mauvaise pratique consistant à ignorer les contrats (ou à ne pas s'en
préoccuper) est assez répandue. Je ne cache pas que l'un des objectifs de cette
série de billets est de combattre cette habitude.

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

En vérité, il existe une troisième façon de s'y prendre. Sous des systèmes
POSIX, on peut déclencher des _coredumps_ par programmation et ce sans
interrompre le cours de l'exécution. Cela peut être fait depuis les
constructeurs de nos exceptions de logique (Voir
[ceci](http://stackoverflow.com/a/979297), ou
[ceci](http://stackoverflow.com/a/18581317)).

### Option 3 : on formalise nos suppositions à l'aide d'assertions

Le C, et par extension le C++, nous offrent un outil tout indiqué pour traquer
les erreurs de programmation : les assertions.

En effet, compilé sans la directive de précompilation `NDEBUG`, une assertion
va arrêter un programme et créer un fichier _core_. Il est ensuite possible
d'ouvrir le fichier _core_ depuis le debuggueur pour pouvoir explorer l'état du
programme au moment de la détection de l'erreur.

#### Exemple d'exploitation des assertions

Sans faire un cours sur gdb, regardons ce qu'il se passe sur ce petit programme :

```c++
// test-assert.cpp
#include <iostream>
#include <cmath>
#include <cassert>
#include <limits>

namespace my {
    /** Computes square root.
     * @pre \c n must be positive, checked with an assertion
     * @post <tt>result * result == n</tt>, checked with an assertion
     */
    double sqrt(double n) {
        assert(n >=0);
        const double result = std::sqrt(n);
        assert(std::abs(result*result - n) < std::numeric_limits<double>::epsilon() * 100);
    }

    /** Computes sinus.
     * @post \c n belongs to [-1, 1], unchecked
     */
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
assertion "n >=0" failed: file "test-assert.cpp", line 14, function: double my::sqrt(double)
Aborted (core dumped)
```

On dispose de suite de l'indication où l'erreur a été détectée.
Mais investiguons plus en avant. Si on lance `gdb ./test-assert core.pid42`
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
assertion "n >=0" failed: file "test-assert.cpp", line 14, function: double my::sqrt(double)

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
#8  0x004011d3 in my::sqrt (n=-1) at test-assert.cpp:14
#9  0x0040125a in main () at test-assert.cpp:32
```

Avec un `up 8` pour se positionner au niveau où l'assertion est fausse, on peut
regarder le code source local avec un `list`, ou de façon plus intéressante,
demander ce que vaut ce fameux `n`.

```
(gdb) up 8
#8  0x004011d3 in my::sqrt (n=-1) at test-assert.cpp:14
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
la précompilation. Généralement, sa définition accompagne le mode _Release_ de
VC++ et de CMake. Ce qui veut dire qu'en mode _Release_ aucune assertion n'est
vérifiée, soit qu'en production, les assertions sont normalement ignorées. Le
corolaire de tout cela est que les assertions sont un outil de vérification de
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
assertions pour détecter statiquement des erreurs de logique. Au contraire, ils
les utilisent pour inhiber l'analyse de certains chemins d'exécutions.

Ainsi, dans l'exemple de `test-assert.cpp` que j'ai donné plus haut, les outils
d'analyse statique de code ne feront pas le rapprochement entre la
post-condition de `my::sin` et la pré-conditon de `my::sqrt`, mais feront
plutôt comme si les assertions étaient toujours vraies, c'est à dire comme si
le code n'appelait jamais `my::sin` avec un nombre négatif.

NB: Je généralise à partir de mon test avec clang analyzer. Peut-être que
d'autres outils savent tirer parti des contrats déclarés à l'aide d'assertions,
ou peut-être le sauront-ils demain.


### propal C++14/17:

Il y a déjà eu des propositions de mots clés plus ou moins sémantiquement forts
pour supporter en standard la PpC en C++. Dans la dernière en date, 
[n3753](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3753.pdf),
John Lakos et Alexei Zakharov introduisent une macro `pre_assert` assez
flexible.

Elle supporte des niveaux d'importance de vérification (toujours, parfois) un
peu à l'image des niveaux Error/Warning/Info/Debug dans les frameworks de log.
Elle permet de faire de la programmation défensive (i.e. de lever des
exceptions au lieu de simples assertions). Elle permet également de transformer
les assertions en assertions de frameworks de tests unitaires.

S'il fallait lui trouver un défaut, je dirai que le nom `pre_assert` est trop
restrictif, en effet cette proposition serait également parfaitement applicable
aux post-conditions et aux invariants.

À noter qu'elle est déjà implémentée et disponible à l'intérieur de la
[bibliothèque BDE/BSL](https://github.com/bloomberg/bde) sous licence MIT.


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
Sébastien Dinot,
Philippe Lacour,
Cédric Poncet-Montange
