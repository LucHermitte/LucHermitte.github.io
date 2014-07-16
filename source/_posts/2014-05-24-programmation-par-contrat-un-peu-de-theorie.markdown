---
layout: post
title: "Programmation par Contrat 1/3"
subtitle: "Un peu de théorie"
date: 2014-05-24 01:00:00 +0100
comments: true
categories: C++
published: true
footer: true
toc: true
language: fr
---

Cela faisait un moment que je voulais partager mes conclusions sur la
_Programmation par Contrat_, et en particulier comment l'appliquer au C++.

Voici un premier billet qui aborde l'aspect théorique. Dans un [second billet]({%post_url 2014-05-28-programmation-par-contrat-les-assertions%}),
je traiterai des _assertions_. En guise de conclusion, je présenterai des
[techniques d'application de la PpC au C++]({%post_url 2014-05-29-programmation-par-contrat-snippets-pour-le-c-plus-plus%})
que j'ai croisées au fil des ans.

## I- Les Erreurs

En développement, il y a toujours des problèmes qui vont venir nous ennuyer.
Certains correspondront à des problèmes plausibles induits par le contexte
(fichiers invalides, connexions réseau coupées, utilisateurs qui saisissent
n'importe quoi, ...), d'autres seront des _erreurs de programmation_.

Dans la suite de ce billet, je vais principalement traiter du cas des erreurs
de programmation. Toutefois, la confusion étant facile, des parenthèses
régulières seront faites sur les situations exceptionnelles mais plausibles.

### I-1. Les types d'erreurs de programmation

Quand on parle d'erreur de programmation, les premières qui vont nous venir à
l'esprit sont les erreurs de syntaxe (point-virgule oublié), ou de grammaire
(types non respectés). Ces erreurs-ci, les langages compilés vont nous les
signaler. On peut considérer qu'il est impossible de livrer un exécutable sur
une plateforme qui n'a pas passé cette phase de vérification.

Il existe de nombreuses autres erreurs de programmation qu'aucun compilateur ne
signalera jamais. On peut se tromper dans la conception ou la retranscription
d'un algorithme, et ainsi renvoyer des résultats numériques aberrants. On peut
aussi faire des suppositions totalement erronées, comme traiter les lignes d'un
fichier qui n'existe pas, ou exploiter un élément après sa recherche
infructueuse dans une liste, ... Les plus classiques sont les accès
hors bornes, et tous les autres problèmes de déréférencement de pointeur nul
et de [_dangling pointer_](http://en.wikipedia.org/wiki/Dangling_pointer).

Bien sûr, un fichier qui n'existe pas est une erreur de contexte. Mais
réaliser un traitement sur un fichier sans vérifier préalablement qu'il existe
est une erreur de programmation. La différence est subtile. J'y reviendrai plus
[loin](#ProgrammationDefensive).

### I-2. Que faire de ces erreurs de programmation ?

Les erreurs qui bloquent la compilation, on n'a pas trop d'autre choix que de
les corriger. Les autres erreurs ... souvent, pas grand chose n'en est fait.
Elles sont là, elles trainent jusqu'à ce qu'elles soient trouvées, puis
corrigées. Les pires d'entre-elles ne sont jamais détectées. C'est souvent le
cas des erreurs numériques, ou des fichiers que l'on croit avoir ouverts.

Dans les meilleurs de mes mondes, on fait en sorte de ne pas pouvoir compiler
quand on est face à une erreur de programmation. Les assertions statiques nous
aideront en cela.  
On peut aussi appliquer des petites recettes dont le principe chapeau consiste
à confier nos invariants au compilateur. Par exemple, on évite de disposer de
variables dans des états non pertinents (_cf._ la
[FAQ C++ de développez](http://cpp.developpez.com/faq/cpp/?page=Les-fonctions#Ou-dois-je-declarer-mes-variables-locales)),
on utilise des références à la place de pointeurs quand on sait que l'on est
censés disposer de _liens_ non nuls, on annote comme **transférables** les
types dont les responsables changent (_cf._ un prochain billet), on fait en
sorte de ne pas pouvoir additionner des distances avec des masses (_cf._
[boost.unit](http://boost.org/libs/units)), ...

Pour les autres cas, [[Meyer1988]](#Meyer1988) a jeté les bases d'un outil, la
_programmation par contrat_. Le C nous offre un second outil, les _assertions_.
Les assertions permettent d'installer des points de contrôle dans un programme
pour vérifier que les traitements se passent bien. Ces points de contrôles
seront utilisés pour vérifier les contrats préalablement définis.
Nous les détaillerons dans
[le prochain billet]({%post_url 2014-05-28-programmation-par-contrat-les-assertions%}).


## II- La programmation par contrat

Les contrats, dans la programmation, servent à poser les bases de qui est censé
faire quoi. Par exemple, la fonction `sqrt(x)` ne prend que des paramètres
numériques positifs _x_, et elle renvoie des nombres toujours positifs qui
vérifient _result = x²_. On retrouve la notion de _domaine de définition_ des
_fonctions_ en mathématiques.

Dit autrement, si on respecte le contrat d'appel d'une fonction (on parle de
ses _pré-conditions_), cette fonction est censée nous garantir respecter son
contrat de sortie (on parle de _post-conditions_). Si les pré-conditions ne
sont pas respectées, les post-conditions (à commencer par le bon déroulement de
la fonction) pourront ne pas être respectées : la fonction est libre de faire
comme elle l'entend.

On peut se demander à quoi ça sert. En effet, si on passe un nombre négatif à
`sqrt` et qu'elle plante, on n'est pas plus avancés. Le bug est toujours là. Et
pourtant, nous avons fait un énorme pas en avant : nous avons formalisé les
contrats de `sqrt`. Nous disposons de spécifications précises, et d'une
[documentation]({%post_url 2014-05-28-programmation-par-contrat-les-assertions%}#Documentation) qui pourra accompagner le code.


Heureusement, nous pouvons aller bien plus loin. Nous pouvons aussi marquer le
code avec des assertions représentatives des contrats identifiés pour repérer
les ruptures de contrats en phases de tests et développement.

Idéalement, nous aurions dû pouvoir aller beaucoup plus loin. En effet, les
outils d'analyse statique de code devraient pouvoir exploiter les contrats
exprimés avec des assertions pour vérifier qu'ils n'étaient jamais violés
lors de leur exploration des chemins d'exécution possibles.  
Seulement, les quelques outils que j'ai pu regarder utilisent au contraire les
assertions pour retirer des branches à explorer.

### II.1- Les trois contrats de la PpC

La PpC définit trois contrats :
#### Les pré-conditions 
Elles sont le pendant des _domaines de définition_ des fonctions mathématiques.
Si l'état du système vérifie les pré-conditions d'une fonction à l'instant de
son appel, alors la fonction est censée se dérouler correctement et de façon
_prévisible_ (je simplifie).

Typiquement, l'_état du système_ correspondra aux paramètres de la fonction,
qu'ils soient explicites, ou implicites (`this`), mais aussi à toutes les
globales accessibles.

#### Les post-conditions 
Les post-conditions sont les garanties que l'on a sur le résultat d'une fonction
si les pré-conditions sont remplies et que la fonction s'est exécutée
correctement.

> __Important :__ Si une fonction voit qu'elle ne pourra pas remplir ses
> post-conditions, alors elle __doit__ échouer -- de préférence en levant une
> exception de _runtime_ en ce qui me concerne.  

Notez cet emploi du _futur_. Il ne s'agit pas de vérifier si les calculs ou
l'algorithme sont corrects en sortie de fonction, mais de vérifier si le
contexte permet bien à la fonction de se dérouler correctement.  

Le cas _"j'ai fait tous mes calculs, ils sont faux, et je ne sais pas
pourquoi"_ ne justifie pas une exception. Il s'agit d'une erreur de
programmation ou de logique.  
Prenons [Vil Coyote](http://fr.wikipedia.org/wiki/Vil_Coyote). Il a un plan
splendide pour attraper Bip Bip -- c'est d'ailleurs la post-condition de son
plan. Il détourne une route pour la faire arriver au pied d'une falaise, et il
peint un tunnel sur le rocher. C'est un algo simple et efficace, Bip Bip
devrait s'écraser sur la roche, et Vil aura son repas. Sauf que. Il y a un bug
avec la peinture qu'il a intégrée (ou avec Bip Bip) : le volatile emprunte le
tunnel. Vous connaissez tous la suite, Vil se lance à sa poursuite et boum. La
post-condition n'est pas respectée car il y a un bug totalement inattendu dans
les pièces que Vil a intégrées. Il n'y avait ici pas de raison de lancer une
exception. La seule exception plausible c'est si Bip Bip venait à ne pas
vouloir emprunter cette route.

Bref, nous le verrons plus loin, et dans le prochain billet, ce cas de bug non
anticipé est mieux traité avec des assertions.

#### Les invariants
Il y a plusieurs natures d'invariants. On va parler d'invariants pour des zones
de codes durant lesquelles une propriété sera vraie :

* un _invariant de boucle_ correspondra à ce qui est toujours vrai à
  l'intérieur de la boucle (p.ex. que `i < N` dans le cas d'une boucle `for`) ;
  [NdA.: À vrai dire, c'est une appellation que l'on peut voir comme abusive. En
  effet, ces invariants peuvent être rompus avant de sortir de la boucle.
  Certains préfèrent utiliser le terme de _variant de boucle_ pour désigner une
  propriété qui va permettre de sortir de la boucle.]
* une variable devrait toujours avoir pour invariant : _est utilisable, et est
  dans un état cohérent et pertinent_ ; cet invariant est positionné à la
  sortie de son constructeur (_cf._ la
  [FAQ C++ développez](http://cpp.developpez.com/faq/cpp/?page=Les-fonctions#Ou-dois-je-declarer-mes-variables-locales)) ;
* un _invariant de classe_ est une propriété toujours observable depuis
  du code extérieur aux instances de la classe -- p.ex. une séquence triée garantira
  que tous les éléments de la séquence sont toujours ordonnés lorsque le code
  utilisant la séquence cherche à y accéder, cependant ponctuellement, le temps
  de l'insertion d'un nouvel élément l'invariant de la classe n'a pas à être
  vérifié depuis les fonctions internes de la séquence ;
* une _référence_ est généralement acceptée en C++ comme un pointeur avec pour
  invariant une garantie de non-nullité.


### II.2- Acteurs et responsabilités

Ces contrats sont définis entre les acteurs qui interviennent dans l'écriture
d'un code.  On peut dans l'absolu distinguer autant d'acteurs que de fonctions.  
Prenons le bout de code suivant :

```c++
double metier() {                  // écrit par l'Intégrateur
   const double i = interrogeES(); // écrit par le responsable UI
   return sqrt(i);                 // écrit par le Mathématicien
}
```

Nous pouvons distinguer trois acteurs : 

* le responsable UI, qui écrit `interrogeES`
* le Mathématicien, qui écrit `sqrt`
* et l'Intégrateur, qui intègre tout cela ensemble lorsqu'il écrit `metier`.

`sqrt` a un contrat simple : le nombre reçu doit être positif. Si l'appel à
`sqrt` échoue (plantage, résultat renvoyé aberrant, ...) tandis que le nombre
passé en paramètre est bien positif, alors le Mathématicien est responsable du
problème et ce peu importe ce qui est fait par les autres acteurs. En effet,
bien que les pré-conditions de `sqrt` sont bien vérifiées, ses post-conditions
ne le sont pas : `sqrt` ne remplit pas sa part du contrat.

Si `i` n'est pas positif, alors le Mathématicien ne peut pas être tenu pour
responsable de quoi que ce soit. La faute incombe au code client de `sqrt`.

À ce stade, tout va dépendre si `interrogeES` dispose d'une post-condition sur
ses sorties du type _renvoie un nombre positif_. Si c'est le cas, la rupture de
contrat est alors à ce niveau, et le responsable UI est responsable de l'erreur
de programmation. En effet, l'Intégrateur est dans son droit d'enchainer
`sqrt(interrogeES())`. C'est exactement la même chose que
`sqrt(abs(whatever))`, personne n'irait accuser l'Intégrateur de ne pas faire
son boulot vu que les pré-conditions de `sqrt` sont censées être assurées par
les post-conditions de `interrogeES`.

En revanche, si `interrogeES` n'a aucune post-condition telle que le nombre
renvoyé sera positif, alors l'Intégrateur est responsable au moment de l'intégration de
s'assurer que ce qu'il va passer à `sqrt` soit bien positif. Une correction
typique serait :

```c++
double metier() {                  // écrit par l'Intégrateur
   const double i = interrogeES(); // écrit par le responsable UI
   if (i <0)
       throw std::runtime_error("invalid input obtained ...");
   return sqrt(i);                 // écrit par le Mathématicien
}
```

Remarquez, que l'Intégrateur est alors face à une erreur de contexte
(/_runtime_) et nullement face à une erreur de programmation. Il est alors en
droit de lever une exception (souvenez-vous, si une post-condition ne peut pas
être respectée, alors la fonction doit échouer), ou de boucler jusqu'à obtenir
quelque chose qui lui permette de continuer. Sans cela nous aurions été face à
une erreur de programmation commise par l'Intégrateur.


En résumé :

> * la responsabilité de vérifier les pré-conditions d'une fonction échoie au
  code client, voire indirectement au code qui alimente les entrées de cette
  fonction appelée.
> * la responsabilité de vérifier les post-conditions d'une fonction échoie à
  cette fonction appelée.


NB: Jusqu'à présent je considérai seulement deux acteurs relativement aux
responsabilités. C'est Philippe Dunski qui m'a fait entrevoir le troisième
intervenant lors de ma relecture de son livre [[Dunksi2014]](#Dunksi2014).

### II.3- Petite parenthèse sur les contrats commerciaux... et les licences

La programmation par contrat n'a pas vocation à avoir des répercutions légales
selon qui ne remplit pas son contrat. Cependant, il y a clairement une
intersection entre la PpC et les responsabilités légales.

Dans le cas où le responsable UI et le Mathématicien sont deux contractants de
l'Intégrateur. Ce que j'ai détaillé au paragraphe précédant est normalement
directement applicable. L'Intégrateur sera responsable vis-à-vis de son client
du bon fonctionnement de l'ensemble, mais le responsable UI et le Mathématicien
ont des responsabilités vis-à-vis de l'Intégrateur.

Si maintenant, le responsable UI ou le Mathématicien ne livrent plus des
[COTS](http://en.wikipedia.org/wiki/Commercial_off-the-shelf) (au sens
commercial), mais des bibliothèques tierces OpenSources ou Libres, à moins que
l'Intégrateur ait pris un contrat de maintenance auprès du responsable UI et du
Mathématicien, il est peu probable que le responsable UI ou le Mathématicien
aient la moindre responsabilité légale vis à vis de l'Intégrateur. 

L'Intégrateur est seul responsable vis-à-vis de son client. À lui de trouver des
contournements, ou mieux de corriger ces composants tiers qu'il a choisi
d'utiliser, et de reverser les patchs à la communauté.

Mais je m'égare, ceci est une autre histoire.  Revenons à nos moutons.

## <a id="ProgrammationDefensive"></a>III- La Programmation Défensive, une philosophie antagoniste ou complémentaire ?

Il est difficile de traiter de la PpC sans évoquer la _Programmation
Défensive_. Souvent ces deux approches sont confondues tant la frontière entre
les deux est subtile.

_Tout d'abord une petite remarque importante, la Programmation Défensive a
d'autres objectifs orthogonaux à ce qui est discuté dans ces billets : elle est
aussi utilisée pour introduire une tolérance aux erreurs matérielles, limiter
les conséquences de ces erreurs (comme les corruptions de mémoire).  C'est un
aspect que je n'aborde pas dans le cadre de la comparaison avec la PpC._

### III.1- Présentons la Programmation Défensive

La _Programmation Défensive_ a pour objectif qu'un programme ne doit jamais
s'arrêter afin de toujours pouvoir continuer. On s'intéresse à la robustesse
d'un programme. 

Bien que la PpC puisse être détournée pour faire de la programmation
défensive, ce n'est pas son objectif premier. La PpC ne fait que stipuler que
si un contrat est respecté, alors tout se passera bien. Si le contrat n'est pas
respecté tout peut arriver : on peut assister à des plantages plus ou moins
prévisibles, on peut produire des résultats erronés, on peut stopper
volontairement au point de détection des erreurs, et on peut aussi remonter
des exceptions. Avec la PpC, on s'intéresse à l'écriture de code correct.

Le choix de remonter des exceptions, depuis le lieu de la détection de la
rupture de contrat, est un choix de programmation défensive. C'est un choix que
j'assimile à une déresponsabilisation des véritables responsables.

Supposons une application qui lit un fichier de distances, et qui pour le besoin
de son métier calcule des racines carrées sur ces distances. L'approche de la 
_programmation défensive_ consisterait à vérifier dans la fonction `my::sqrt` que
le paramètre reçu est positif, et à lever une exception dans le cas contraire.

Ce qui donnerait :

```c++
double my::sqrt(double n) {
    if (n<0) throw std::domain_error("Negative number sent to sqrt");
    return std::sqrt(n);
}

void my::process(boost::filesystem::path const& file) {
    boost::ifstream f(file);
    if (!f) throw std::runtime_error("Cannot open "+file.string());
    double d;
    while (f >> d) {
        my::memorize(my::sqrt(d));
    }
}
```

Si un nombre négatif devait être présent dans le fichier, nous aurions droit à
l'exception *"Negative number sent to sqrt"*. Limpide, n'est-ce pas ? On ne sait
pas quel est le nombre, ni d'où il vient. Après une longue investigation pour
traquer l'origine de ce nombre négatif, on comprend enfin qu'il faut
instrumenter `process` pour intercepter l'exception. Soit on fait le `catch` au
niveau de la fonction, et on sait dans quel fichier a lieu l'erreur, soit on
encadre l'appel à `my::sqrt` pour remonter plus d'informations.

```c++
void my::process(boost::filesystem::path const& file) {
    boost::ifstream f(file);
    if (!f) throw std::runtime_error("Cannot open "+file.string());
    double d;
    while (f >> d) {
        double sq = 0;
        try {
            sq = my::sqrt(d);
        }
        catch (std::logic_error const&) {
            throw std::runtime_error(
                "Invalid negative distance " + std::to_string(d)
                +" at the "+std::to_string(l)
                +"th line in distances file "+file.string());
        }
        my::memorize(sq);
    }
}
```

Et là ... on fait ce que le code client aurait dû faire dès le début : assurer
que le contrat des fonctions appelées est bien respecté.  
En effet, si on avait embrassé la PpC dans l'écriture de ces deux fonctions, ce
bout de code aurait ressemblé à :

```c++
double my::sqrt(double n) {
    assert(n>=0 && "sqrt can't process negative numbers");
    return std::sqrt(n);
}

void my::process(boost::filesystem::path const& file) {
    boost::ifstream f(file);
    if (!f) throw std::runtime_error("Cannot open "+file.string());
    double d;
    for (std::size_t l = 1 ; f >> d ; ++l) {
        if (d <= 0) 
            throw std::runtime_error(
                "Invalid negative distance " + std::to_string(d)
                +" at the "+std::to_string(l)
                +"th line in distances file "+file.string());
        my::memorize(my::sqrt(d));
    }
}
```
Cela n'est-il pas plus simple et propre pour disposer d'un message non
seulement plus explicite, mais surtout bien plus utile ? Comparez ce nouveau
message *"Invalid negative distance -28.15 at the 42th line of distances file
distances.txt"*, au précédent *"Negative number sent to sqrt"*.  
Notez que l'on pourrait aussi critiquer l'impact en termes de performances de
la solution précédente (avec le `catch`). Un `catch` n'est pas si gratuit que
cela -- a contrario du *Stack Unwinding*.

### III.2- Des objections ?

Il est des objections classiques à l'utilisation de la PpC en terrain où la
Programmation Défensive occupe déjà la place. Décortiquons-les.

#### *"-On utilise l'une ou l'autre"*
Oui et non. Si la PpC s'intéresse à l'écriture de code correct, la
Programmation Défensive s'intéresse à l'écriture de code robuste. 
L'objectif premier n'est pas le même (dans un cas on essaie de repérer et
éliminer les erreurs de programmation, dans l'autre on essaie de ne pas planter
en cas d'erreur de programmation), de fait les deux techniques peuvent se
compléter.  
D'abord on élimine les bugs, ensuite on essaie de résister aux bugs
récalcitrants.

À vrai dire, on peut utiliser simultanément ces deux approches sur de mêmes
contrats. En effet, il est possible de modifier la définition d'une assertion
en mode _Release_ pour lui faire lever une exception de logique. En mode
_Debug_ elle nous aidera à contrôler les enchainements d'opérations

Ce qui indubitable, c'est qu'en cas de certitude qu'il n'y a pas d'erreur de
programmation sur des enchainements de fonctions, alors il n'y a pas besoin de
test dynamique sur les entrées des fonctions.  
Reste que toute la difficulté réside dans comment être certains qu'une séquence
d'opérations est exempte de bugs.

#### *"- La PpC éparpille les vérifications alors que la Programmation Défensive les factorise."*
Il est vrai que la Programmation Défensive permet d'une certaine façon de
centraliser et factoriser les vérifications. Mais les vérifications ainsi
centralisées ne disposent pas du contexte qui permet de remonter des erreurs
correctes. Il est nécessaire d'enrichir les exceptions pauvres en les
transformant au niveau du code client, et là on perd les factorisations.  
D'où la question légitime que l'on est en droit de se poser : *"Mais pourquoi ne
pas faire ce que le code client était censé faire dès le début ? Pourquoi ne
pas vérifier les pré-conditions des fonctions que l'on va appeler, avant de les
appeler ?"*

Ensuite, il est toujours possible de factoriser grâce aux assertions. Si en
mode _Release_ les assertions lèvent des exceptions, alors factorisation il y
a.

Ce qui me gêne avec cette _factorisation_, c'est que l'on mélange les problèmes
de _runtime_ avec les erreurs de programmation ou de logique. J'aime bien le
[_Single Responsability Principle (SRP)_](http://en.wikipedia.org/wiki/Single_responsibility_principle),
mais là, j'ai la franche impression que l'on mélange les responsabilités des
vérifications.  
De fait, on commence à avoir des systèmes aux responsabilités de plus en plus
confuses.

De plus, cette factorisation implique de toujours vérifier dynamiquement ce qui
est garantit statiquement. D'autant que idéalement s'il n'y a pas d'erreur de
programmation, alors il n'y a pas de test à faire dans les cas où le _runtime_
n'a pas à être vérifié.  
Quel sens il y a-t-il à écrire ceci ? 

```c++
for (std::size_t i=0, N=vect.size(); i!=N ; ++i)
    f(vect.at(i));

// ou de vérifier la positivité des paramètres de sqrt() dans
sqrt(1-sin(x))
```

#### *"-Le mode Debug ne doit pas se comporter différemment du mode Release!"*
Remontons à l'origine de cette exigence pour mieux appréhender son impact sur
la PpC telle que je vous la propose (avec des assertions).  

Parfois, le mode _Debug_ est plus permissif que le mode _Release_ : il cache
des erreurs de programmation. Souvent c'est du à des outils (comme VC++) dont le mode
_Debug_ zéro-initialise des variables même quand le code néglige de les
initialiser.

Avec des assertions, c'est tout le contraire. En effet, le mode _Debug_ ne sera
pas plus permissif, mais au contraire il sera plus restrictif et intransigeant
que le mode _Release_. Ainsi, si un test passe en mode _Debug_, il passera
également en mode _Release_ (relativement aux assertions) : si le test est OK,
c'est que les assertions traversées ne détectent aucune rupture de contrat en
_Debug_, il n'y aurait aucune raison qu'il en aille autrement en _Release_.
A contrario, un test qui finit en _coredump_ en _Debug_ aurait pu tomber en
marche en _Release_, comme planter de façon plus ou moins compréhensible (plutôt
moins en général).  
Ce qui est sûr, c'est qu'en phases de développement et de tests, les
développeurs auraient vu l'erreur de programmation et ils auraient dû la
corriger pour voir le test passer.

#### *"- La programmation Défensive est plus adaptée aux développeurs inexpérimentés."*
C'est possible. On ne réfléchit pas avant. On code et on voit ensuite ce qu'il
se passe. Traditionnellement, les débutants tendent à être formés de la sorte.

Seulement, on complexifie grandement la base de code avec cette approche.
Les erreurs (de programmations et logiques) sont mélangées aux cas dégradés du
_runtime_. Nous avons une vision plus floue, des fonctions plus complexes qui
propagent et rattrapent des exceptions qui ne sont pas censées se produire.  
Bref, nous avons une logique d'ensemble plus difficile à maîtriser.

Les cas dégradés induits par nos métiers complexifient déjà grandement les
applications. Rajouter, au milieu de cela, du code pour gérer les erreurs de
programmation complexifie encore plus les systèmes. D'ailleurs, ne
rajoutent-ils pas de nouveaux risques de bugs ?

De fait, je me pose sincèrement la question : en voulant rendre plus
accessibles nos systèmes à des développeurs inexpérimentés, ne faisons-nous pas
le contraire ?

À noter aussi aussi que le diagnostic des erreurs de _runtime_ ou de logique
est plus pauvre avec la _factorisation_ de la Programmation Défensive. Et de
fait, on complexifie les tâches d'investigation des problèmes vu que l'on
déresponsabilise les véritables responsables.

### III.3- En résumé
Sinon, voici mes conclusions personnelles sur le sujet :

* La PpC s'intéresse à l'écriture de codes corrects. La Programmation Défensive
  s'intéresse à l'écriture de codes qui restent robustes dans le cas où ils ne
  seraient pas corrects.
* Philosophiquement, je préfère 100 fois la PpC à la Programmation Défensive :
  il faut assumer nos responsabilités et ne pas décharger nos utilisateurs de
  leurs devoirs.
* Toutefois, il est possible de détourner la PpC  basée sur des assertions en C
  et C++ pour faire de la Programmation Défensive ; p.ex. l'assertion pourrait
  être détournée en *Release* pour lever une exception. J'y reviendrai dans le
  [prochain billet]({%post_url 2014-05-28-programmation-par-contrat-les-assertions%}).


### III.4- Comment reconnaitre des contrats ?

Il est important de le rappeler, les contrats tels que présentés ici sont
orientés vers la recherche des erreurs de programmation. C'est à dire, un code
qui ne respecte pas les contrats de ses divers constituants présente une erreur
de programmation.

En aucun cas une violation de contrat correspondra à une situation
exceptionnelle (et plausible), _cf._ [[Wilson2006]](#Wilson2006)

Il est également à noter qu'une vérification de contrat devrait pouvoir être
retirée d'un code source sans que son comportement ne soit impacté. En effet,
un programme dépourvu d'erreur de logique n'aura aucun contrat qui se fasse
violer, et la vérification devient superflue.

## IV- Parenthèse OO : PpC & _Principe de Substitution de Liskov_ (LSP)

Je ne rentrerai pas dans les détails du LSP. Je vous renvoie plutôt à la
[FAQ C++ de développez](http://cpp.developpez.com/faq/cpp/?page=L-heritage#Qu-est-ce-que-le-LSP),
ou à [[Dunksi2014]](#Dunksi2014). Il faut retenir que le LSP est un outil qui
permet d'éviter de définir des hiérarchies de classes qui se retourneront
contre nous.

Le LSP est formulé relativement aux contrats des classes pour établir quand une
classe peut dériver (publiquement en C++) en toute quiétude d'une autre.
Le principe est que :

* les pré-conditions ne peuvent être qu'affaiblies, ou laissées telles quelles,
* les post-conditions ne peuvent être que renforcées, ou laissées telles
  quelles,
* et une classe fille ne peut qu'ajouter des invariants.

Dit comme cela, cela peut paraitre abscons, et pourtant c'est très logique.  

####Quelques exemples
Prenons par exemple, une compagnie aérienne. Elle a des pré-requis sur les
bagages acceptés sans surcouts. Pour toutes les compagnies, un bagage de
50x40x20cm sera toujours accepté. En particulier, chez les compagnies
low-costs. En revanche, les grandes compagnies historiques (et non low-costs)
affaiblissent cette pré-condition : on peut s'enregistrer avec un bagage
bien plus gros sans avoir à payer de supplément (certes il partira en soute).  
Il en va de même pour les post-conditions : nous n'avons aucune garantie
d'estomac rempli sans surcouts une fois à bord de l'avion. Sauf chez les
compagnies traditionnelles qui assurent en sortie un estomac non vide.  
On peut donc dire a priori qu'une _compagnie low-cost_ est une _compagnie
aérienne_, de même qu'une _compagnie traditionnelle_ est une _compagnie
aérienne_.

Côté invariants, un rectangle immuable a tous ses côtés perpendiculaires, un
carré immuable a en plus tous ses côtés de longueur égale.

Parmi les conséquences du LSP, on pourra déduire qu'une _liste triée_ n'est pas
substituable à une _liste_, ou qu'un _carré non immuable_ n'est pas un
_rectangle non immuable_. Je vous renvoie à la littérature et/ou la FAQ pour
plus d'informations sur le sujet.

## V- Remerciements

Un grand merci à tous mes relecteurs, correcteurs et détracteurs. J'ai nommé: 
Julien Blanc,
Guilhem Bonnefille,
David Côme,
Sébastien Dinot,
Iradrille,
Philippe Lacour,
Cédric Poncet-Montange

## VI- Références

[_NdA: Je réorganiserai les liens au fur et à mesure des sorties des articles_]

- <a id="Meyer1988"></a>[Meyer2000] -- [_Conception et programmation orientées objet_](http://www.editions-eyrolles.com/Livre/9782212122701/conception-et-programmation-orientees-objet) de Bertrand Meyer, Eyrolles, 1988, Seconde Édition parue en 2000
- <a id="Dunksi2014"></a>[Dunksi2014] -- [_Coder Efficacement -- Bonnes pratiques et erreurs à éviter (en C++)_](http://www.d-booker.fr/programmation-et-langage/157-coder-efficacement.html) de Philippe Dunski, D-Booker, Février 2014
- <a id="IPCpp"></a>[Wilson2004] -- _Imperfect C++_ de Matthew Wilson, Addisson-Wesley Professionnal, Octobre 2004.
- <a id="Wilson2006"></a>[Wilson2006] -- [_Contract Programming 101_](http://www.artima.com/cppsource/deepspace3.html), Matthew Wilson, artima, Janvier 2006.
- [n3753](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3753.pdf)
- Billets d'Andrzej  
  Il s'agit là d'une excellente série d'articles/billets sur la PpC.
  - http://akrzemi1.wordpress.com/2013/01/04/preconditions-part-i/
  - http://akrzemi1.wordpress.com/2013/02/11/preconditions-part-ii/
  - http://akrzemi1.wordpress.com/2013/03/13/preconditions-part-iii/
  - http://akrzemi1.wordpress.com/2013/04/18/preconditions-part-iv/
- Autres articles
  - [_When and How to Use Exception_](http://www.drdobbs.com/when-and-how-to-use-exceptions/184401836), Herb Sutter, August 01, 2004
  - http://blog.regehr.org/archives/1091
  - http://pempek.net/articles/2013/11/16/assertions-or-exceptions/
  - http://pempek.net/articles/2013/11/17/cross-platform-cpp-assertion-library/
  - [_Programmation par contrat, application en C++_](http://julien-blanc.developpez.com/articles/cpp/Programmation_par_contrat_cplusplus/), de Julien Blanc, 14 décembre 2009  
    Vous trouverez d'autres explications et d'autres techniques dans son
    article.


