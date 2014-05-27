---
layout: post
title: "Programmation par contrat 2/3 -- les assertions"
date: 2014-05-26 12:07:42 +0200
comments: true
categories: C++
published: false
footer: true
---

Dans ce second billet sur la _Programmation par Contrat_, nous allons voir que
faire des contrats une fois établis, et en particulier je vais vous présenter
un outil dédié à la détection les erreurs de programmation : les _assertions_.

## I- <a id="Documentation"></a>Documentation

Comme je l'avais signalé dans le précédent billet, la première chose que l'on
peut faire à partir des contrats, c'est de les documenter clairement.
Il s'agit probablement d'une des choses les plus importantes à documenter dans
un code source. Et malheureusement trop souvent c'est négligé.

L'outil [Doxygen](http://doxygen.org) met à notre disposition les tags `@pre`,
`@post`, et `@invariant` pour documenter nos contrats. Je ne peux que
vous conseiller d'en user et d'en abuser.


## II- Comment prendre en compte les contrats dans le code ?

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
problèmes de conceptions et de déresponsabilisation évoqués dans le
[billet précédent]({%post_url 2014-05-24-programmation-par-contrat-un-peu-de-theorie%}),
il y a un soucis technique.

En effet, en temps normal avec une exception en C++, on ne peut rien avoir de
mieux que des informations sur le lieu de la détection (_i.e._ :` __FILE__` et
`__LINE__`). Et encore faut-il disposer de classes _exception_ conçues pour
stocker une telle information ; ce n'est par exemple pas le cas des
`std::logic_error` qui sont levées depuis des fonctions comme
`std::vector<>::at()`.

Par _"rien de mieux que le lieu de la détection"_, il faut comprendre que l'on
ne disposera d'aucune autre information de contexte. En effet, une exception
remonte jusqu'à un `catch` compatible ; or à l'endroit du `catch`, on ne peut
plus avoir accès à l'état (de toutes les variables, dans tous les threads, ...)
au moment de la détection du problème.

En vérité, il y existe deux moyens peu ergonomiques pour y avoir accès. Le
premier consiste à mettre des points d'arrêt sur les levers ou les
constructions d'exceptions, et à exécuter le programme depuis un débuggueur. Le
second consiste à supprimer du code source tous les `catchs` qui sont
compatibles avec l'erreur de logique.

Aucune de ces deux options n'est véritablement envisageable pour des tests
automatisés ; et la seconde l'est encore moins pour du code qui va aller en
production. Ces options sont en revanche envisageables pour investiguer. 

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
[celà](http://stackoverflow.com/a/18581317)).


### Option 3 : on formalise nos suppositions à l'aide d'assertions

Le C, et par extension le C++, nous offrent un outil tout indiqué pour traquer
les erreurs de programmation : les assertions.

En effet, compilé sans la directive de précompilation `NDEBUG`, une assertion
va arrêter un programme et créer un fichier _core_. Il est ensuite possible
d'ouvrir le fichier _core_ depuis le débuggueur pour pouvoir explorer l'état du
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
        return result;
    }

    /** Computes sinus.
     * @post \c n belongs to [-1, 1], checked with an assertion
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
#9  0x0040125a in main () at test-assert.cpp:33
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


N.B.: l'équivalent existe pour d'autres environnements comme VC++.

#### <a id="Phases"></a>Un outil pour les phases de développement et de tests ...

Je vais paraphraser [[Wilson2006] §1.1.](Wilson2006), qui énonçait déjà des
évidences : _"Plus tôt on détecte une erreur, mieux c'est"_.  C'est un adage
que vous devez déjà connaitre. Concrètement, cela veut dire que l'on va
préférer trouver nos erreurs, dans l'ordre :

1. lors de la phase de conception,
2. lors la compilation,
3. lors de l'analyse statique du code
4. lors des tests unitaires,
5. lors des tests en _debug_,
6. en pré-release/phase béta,
7. en production.

Je traiterai rapidement de la phase 2. de compilation en
[fin de ce billet](#VerificationsStatiques).  
Les assertions pour leur part interviennent lors des phases 4. et 5.

Les assertions ne sont vérifiées que si `NDEBUG` n'est pas défini au moment de
la précompilation. Généralement, sa définition accompagne le mode _Release_ de
VC++ et de CMake. Ce qui veut dire qu'en mode _Release_ aucune assertion n'est
vérifiée. Soit qu'en production, les assertions sont normalement ignorées. Le
corolaire de tout cela est que les assertions sont un outil de vérification de
la logique applicative qui n'est utilisé qu'en phases de développement et de
tests.

Ce n'est certes pas le plus tôt que l'on puisse faire, mais c'est déjà quelque
chose qui intervient avant que des utilisateurs manipulent le produit final.

#### ... voire de production

Bien que les assertions ne soient censées porter que sur ces phases 4. et 5., il
est possible de les détourner en phases 6. et 7. pour tenter de rendre plus
robuste le produit en le faisant résister aux erreurs de programmation qui ont
échappé à notre vigilance lors des phases précédentes.

On entre dans le royaume de la _Programmation Défensive_ que j'ai déjà
abondamment décrit.

Comment peut-on détourner les assertions ? Tout simplement en détournant leur
définition. N'oublions pas que les assertions sont des macros dont le
comportement exact dépend de la définition de `NDEBUG`.

Une façon assez sale de faire serait p.ex.:
```c++
#if defined(NDEBUG)
#   define my_assert(condition_, message_) \
       if (!(condition_)) throw std::logic_error(message_)
#else
#   define my_assert(condition_, message_) \
       assert(condition_ && message_)
#endif
```


#### Techniques connexes

Il est possible de rendre les messages produits par `assert` un petit peu plus
compréhensibles en profitant du fonctionnement interne de la macro.

Exemples :
```c++
assert(n>=0 && "sqrt can't process negative numbers");
```

```c++
switch (myEnum) {
    case Enum::first: .... break;
    case Enum::second: .... break;
    default:
        assert(!"Unexpected case");
}
```

#### Exploitation des assertions par les outils d'analyse statique de code

Les outils d'analyse statique de code comme
[clang analyzer](http://clang-analyzer.llvm.org/) sont très intéressants. En
plus, ils interviennent en [phase 3](#Phases)! Seulement, à mon grand regret,
ils ne semblent pas exploiter les assertions pour détecter statiquement des
erreurs de logique.
Au contraire, ils utilisent les assertions pour inhiber l'analyse de certains
chemins d'exécution.

Ainsi, dans l'exemple de `test-assert.cpp` que j'ai donné plus haut, les outils
d'analyse statique de code ne feront pas le rapprochement entre la
post-condition de `my::sin` et la pré-condition de `my::sqrt`, mais feront
plutôt comme si les assertions étaient toujours vraies, c'est à dire comme si
le code n'appelait jamais `my::sqrt` avec un nombre négatif.

N.B.: Je généralise à partir de mon test avec _clang analyzer_. Peut-être que
d'autres outils savent tirer parti des contrats déclarés à l'aide d'assertions,
ou peut-être le sauront-ils demain.  
Pour information, je n'ai pas eu l'occasion de tester des outils comme _Code
Contract_ (pour .NET), Ada2012 (si on sort du périmètre du C++) ni même
_Polyspace_.


## III- Le standard s'enrichira-t-il en 2017 pour programmer avec des contrats ?

Il y a déjà eu des propositions de mots clés plus ou moins sémantiquement forts
pour supporter la PpC en standard en C++. Dans la dernière en date, 
[n3753](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3753.pdf),
John Lakos et Alexei Zakharov introduisent un ensemble de macros `pre_assert`
assez flexible.

Elles supportent des niveaux d'importance de vérification (_toujours_,
_parfois_) un peu à l'image des niveaux _Error_/_Warning_/_Info_/_Debug_ dans
les frameworks de log.  Elle permet de faire de la programmation défensive
(i.e. de lever des exceptions au lieu de simples assertions). Elle permet
également de transformer les assertions en assertions de frameworks de tests
unitaires.

S'il fallait lui trouver un défaut, je dirai que sémantiquement le nom
`pre_assert` est trop restrictif, en effet cette proposition serait également
parfaitement applicable aux post-conditions et aux invariants.

À noter qu'elle est déjà implémentée et disponible à l'intérieur de la
[bibliothèque BDE/BSL](https://github.com/bloomberg/bde) sous licence MIT.


## IV- <a id="VerificationsStatiques"></a>Invariants statiques

Pour conclure, il est important de remarquer que certains contrats peuvent être
retranscrit de manière plus forte qu'une assertion qui ne sera vérifiée qu'en
phase de tests.

En effet, le compilateur peut en prendre certains à sa charge.

#### Les assertions statiques sont nos amies
Elles sont beaucoup utilisées lors de l'écriture de classes et fonctions
génériques pour s'assurer que les arguments _templates_ vérifient certaines
contraintes.  
Mais c'est loin d'être le seul cas d'utilisation. Je m'en sers
également pour vérifier que j'ai autant de chaines de caractères que de valeurs
dans un énuméré. Avec mes [plugins pour vim](https://code.google.com/p/lh-vim/wiki/lhCpp),
je génère automatiquement ce genre de choses avec `:InsertEnum MyEnum one two
three` :

```c++
// .h
... includes qui vont bien
struct MyEnum {
    enum Type { one, two, three, MAX__, UNDEFINED__, FIRST__=0 };
    ...
    MyEnum(std::string const& s);
    char const* toString() const;
    ...
private:
    Type m_value;
};

// .cpp
... includes qui vont bien
namespace  { // Anonymous namespace
    typedef char const* const* strings_iterator;
    static char const* const MYENUM_STRINGS[] =
    { "one", "two", "three" };
} // Anonymous namespace
...
char const* MyEnum::toString() const
{
    // v-- Ici se trouve l'assertion statique
    static_assert(MAX__ == std::extent<decltype(::MYENUM_STRINGS)>::value, "Array size mismatches number of elements in enum");
    assert(m_value != UNDEFINED__); // Yes, I know UNDEFINED__ > MAX__
    assert(m_value < MAX__);
    return MYENUM_STRINGS[m_value];
}
```

#### Préférez les références aux pointeurs
Dans la signature d'une fonction, les références posent une pré-condition : la
valeur passée en paramètre par référence doit être non nulle -- à charge au
code client de vérifier cela.

Dans le corps de la fonction, elles deviennent pratiquement un invariant : à
partir de là, il est certain que la chose manipulée indirectement est censée
exister. Il n'y a plus besoin de tester un pointeur, que cela soit avec une
assertion (PpC), ou avec un test dynamique (Programmation Défensive).  
Certes, cela ne protège pas des cas où la donnée est partagée depuis un
autre thread où elle pourrait être détruite.

John Carmack recommande leur utilisation (en place de pointeurs) dans un
[billet sur l'analyse statique de code](http://www.altdevblogaday.com/2011/12/24/static-code-analysis/)
publié sur #AltDevBlog.

#### boost.unit
[boost.unit](http://boost.org/libs/units) est le genre de bibliothèque qui
aurait pu sauver une fusée. L'idée est de ne plus manipuler de simples valeurs
numériques, mais des quantités physiques. Non seulement on ne peut pas additionner des
masses à des longueurs, mais en plus l'addition de masses va prendre en compte
les ordres de grandeur.  
Bref, on type fortement toutes les quantités numériques selon les unités du
[Système International](http://fr.wikipedia.org/wiki/Syst%C3%A8me_international_d%27unit%C3%A9s).

#### Une variable devrait toujours être pertinente et utilisable
Un objet devrait toujours avoir pour invariant : _est dans un état pertinent et
utilisable_. Concrètement, cela implique deux choses pour le développeur.

1. Un tel invariant se positionne à la sortie du constructeur de l'objet.
2. On doit retarder la définition/déclaration d'une variable jusqu'à ce que
   l'on soit capable de lui donner valeur pertinente, et préférentiellement
   définitive.

Un futur point de la FAQ C++ de développez traitera plus en détails de cela. Je
donnerai le lien quand nous aurons finalisé cette entrée.  
_[N.B.: pour ceux qui ont les droits d'accès, la discussion se passe par [là](http://www.developpez.net/forums/d1422877-2/c-cpp/cpp/priv-ressources-cpp/faq-cpp/dois-declarer-variables-locales/).]_

#### Corolaire : préférez les constructeurs aux fonctions `init()` et autres _setters_
Dans la continuité du point précédent, il faut éviter toute initialisation qui
se produit après la construction d'un objet. En effet, si l'objet nécessite
deux appels de fonction pour être correctement initialisé, il y a de grands
risques que le second appel soit purement et simplement oublié. Il faut de fait
tester dynamiquement dans chaque fonction de l'objet s'il a bien été initialisé
avant de tenter de s'en servir.

Si le positionnement de l'invariant d'_utilisabilité_ se fait en sortie du
constructeur, nous aurions à la place la garantie que soit l'objet existe et
il est utilisable, soit l'objet n'existe pas et aucune question ne se pose,
nulle part. 

#### Choisir le bon type de pointeur

Avec le C++11 nous avons l'embarras du choix pour choisir comment manipuler des
entités ou des données dynamiques. Entre, `std::unique_ptr<>`,
`std::shared_ptr`, `boost::ptr_vector`, les références, les pointeurs bruts
(/nus), `std::optional<>` (C++14), _etc._, on peut avoir l'impression que c'est
la jungle.

Quel rapport avec les invariants statiques ? Et bien, comme pour les références
`std::unique_ptr<>`, apporte une garantie supplémentaire par rapport à un
simple pointeur brut.
Ce type assure que la fonction qui réceptionne le pointeur en devient
responsable alors que l'émetteur s'est débarrassé de la patate chaude. Et le
compilateur est là pour entériner la transaction et faire en sorte que les deux
intervenants respectent bien ce contrat de passation de responsabilité.

Je pense que j'y reviendrai dans un prochain billet. En attendant, je ne peux
que vous conseiller la lecture de cette
[présentation](http://exceptionsafecode.com/slides/svcc/2013/shared_ptr.pdf)
assez exhaustive d'Ahmed Charles.

