---
layout: post
title: "Le C++ Moderne"
date: 2012-04-04 10:44:47 +0100
comments: true
categories: C++
published: true
footer: true
---
_Billet initialement posté sur [mon blog du boulot](https://thor.si.c-s.fr/blogs/cs2/#urn:md5:a7dc48a340cd43c09c1180dd99234483)_


Suite à la grille que j'avais donnée dans mon [précédent billet](https://thor.si.c-s.fr/blogs/cs2/#urn:md5:4bdf150032f41bbea81a4ee4a259d522), deux questions fort pertinentes m'ont été posées :

> Que signifie «simple» ?
> De même «C++ moderne» concerne la syntaxe ?

## De la simplicité

_Hum ... est-il simple de définir la simplicité ? Voyons voir. Ah! Même la page
wiki du [principe du KISS](http://en.wikipedia.org/wiki/KISS_principle) n'élabore pas sur le sujet. Bon._

J'estime qu'un code est simple quand il résout, correctement, un problème en
peu de lignes, et qu'il demande peu d'énergie pour comprendre ce qu'il fait des
mois plus tard quand on a besoin de le relire, voire de le maintenir.

Un exemple facile serait par exemple un code C qui lit depuis un fichier un
nombre de lignes inconnu à l'avance et le même code en C++. La version robuste
(qui prend en compte les éventuels problèmes) est vite complexe en C. En effet,
en C il faut gérer manuellement les réallocations pour chaque ligne lue, mais
aussi pour le tableau de lignes. Le C++, mais aussi glibc, fournissent des
_primitives_ dédiées qui épargnent au développeur de devoir réinventer la roue.
Cf l'article de Bjarne Stroustrup: [Learning C++ as a new language](http://www.research.att.com/~bs/new_learning.pdf).

On touche au paradoxe de la simplicité entre le C et le C++. Le C qui ne
dispose uniquement que des briques élémentaires (relativement à la gestion de
la mémoire et des chaines -- et encore) est plus simple que le C++. Pourtant le
C++ qui offre des encapsulations de ces briques élémentaires permet de produire
plus simplement du code robuste.

Quel est le rapport avec les bibliothèques C++ de manipulation de documents XML
? Et bien, je vous invite à comparer la manipulation de chaines de caractères
de Xerces-C++ avec les autres bibliothèques plus modernes.

## Le C++ Moderne

Pour comprendre ce qu'est le _C++ Moderne_, il faut d'abord voir ce qu'est le _C++ historique_.

### Le C++ Historique

Un majorité écrasante, et regrettable, de ressources pédagogiques sur le C++
suit ce que l'on appelle aujourd'hui une approche historique. « Le C++ descend
du C, il est donc logique d'enseigner le C avant le C++ ». Après tout nous
enseignons le latin avant le français à nos enfants, non ? Certes, cette
comparaison, comme bien des comparaisons, est fallacieuse, mais posons-nous
tout de même la question : où est le mal à enseigner itérativement du C vers le
C++ ? Au delà de l'aspect pédagogique qui nous fournit des abstractions plus
simples à manipuler sur ce plan pédagogique, le soucis est dans les habitudes
qui seront prises.

Le C++ historique est un C++ où la bibliothèque standard ne mérite pas mieux
qu'une note en annexe d'un cours, chose qui pousse à réinventer la roue et à
verser dans le syndrome du
[NIH](http://en.wikipedia.org/wiki/Not_invented_here). C'est un C++ dont les
idiomes sont maîtrisés approximativement – assez logiquement, les cours
modernisés sont plus au fait de l'état de l'art en matière d'idiomes C++. Mais
c'est aussi et surtout un C++ où la gestion des erreurs est confiée à des codes
de retour, comme en C.

Souvent nous le savons que trop bien, le développeur est vite laxiste et ne
teste pas toutes les fonctions qui peuvent échouer pour traiter les cas
dégradés. À commencer par les erreurs de type « mémoire saturée ». Un tel code
cavalier dans sa gestion des erreurs ressemblerait à ceci :

```cpp
    NotifyIcon* CreateNotifyIcon()
    {
        NotifyIcon* icon = new NotifyIcon();
        icon.set_text("Blah blah blah");
        icon.set_icon(new Icon(...), GetInfo());
        icon.set_visible(true);
        return icon;
    }
```

Sauf que … le C++ peut lever des exceptions. C'est le comportement par défaut
des allocations de mémoire en C++, des types standards qui nous simplifient
grandement la gestion des chaînes de caractères et des tableaux
redimensionnables, des listes chaînées, des tables associatives, etc. Des COTS
peuvent aussi lever des exceptions à notre insu. Les exceptions doivent donc
être prises en compte. De plus, il est envisageable que plusieurs des fonctions
invoquées ci-dessus puissent échouer, le code précédent ne le prenait pas en
compte. Supposons que les échecs soient notifiés par des exceptions, et tâchons
de corriger le code précédent.

Une version corrigée pourrait ressembler à ceci :

```cpp
    NotifyIcon* CreateNotifyIcon()
    {
        NotifyIcon* icon = new NotifyIcon();
        try {
            icon.set_text("Blah blah blah");
            icon.set_visible(true);
            Info info = GetInfo();
            icon.set_icon(new Icon(...), info);
        } catch (...) {
            delete icon; throw;
        }
        return icon;
    }
```

Il semblerait que nous ayons fini. Et pourtant ce tout petit code est juste inmaintenable.

Que se passe-t-il si `set_icon` lève une exception ? Sommes nous certains que l'icône passée sera bien libérée ?
Pouvons-nous changer de place sans risques le `set_icon` ? Même si un jour la copie du `GetInfo` lève à son tour une exception ?
Et si nous rajoutions une troisième ressource, comment faire pour nettoyer correctement derrière nous ?
Bienvenu dans l'enfer de la gestion des ressources et du traitement des cas
dégradés du C/C++ ! On aurait pu croire que ce code anodin soit simple à
corriger avec un petit _catch_, ce n'est pourtant pas le cas.

NB: Ces codes proviennent de deux articles, un de Raymond Chen, et sa réponse
par Aaron Lahman, au sujet de l'audit de codes dont le sujet est de savoir quel
style est le plus propice à repérer rapidement des codes incorrects. La
traduction de la réponse est disponible à l'adresse suivante :
[http://alexandre-laurent.developpez.com/cpp/retour-fonctions-ou-exceptions/](http://alexandre-laurent.developpez.com/cpp/retour-fonctions-ou-exceptions/).

Vous trouverez dans l'article une version corrigée du code qui repose sur les
codes de retour, avec un `if` toutes les deux lignes en moyenne pour remonter
les erreurs, et restituer les ressources.

### <a id="C++Moderne"></a>Le C++ Moderne

La solution aux problèmes du C++ historique réside dans le C++ moderne.
Décryptons cette tautologie.

Oui le C++ est extrêmement complexe, personne ne prétend d'ailleurs le
maîtriser dans sa totalité, et l'avènement du C++11 n'est pas fait pour
améliorer les choses. Et pourtant, paradoxalement le C++ est plus simple à
utiliser que ce que l'on peut croire. Il s'agit d'accepter de revoir notre
façon de penser la gestion des cas dégradés. Là où la tradition nous pousse à
envisager tous les chemins d'exécution possibles, ce qui a vite fait
d'exploser, l'approche moderne nous pousse à surveiller toutes les ressources
qui devront être restituées.

Pour cela on a recourt à une spécificité du C++ : tout objet local sera
implicitement détruit en sortie de la portée où il vit, et ce quelque soit le
chemin (propre -- i.e. suite à un return ou une exception levée) qui conduit à
l'extérieur de cette portée. Si l'on veut être pédant, ce comportement
déterministe répond à l'appellation _Resource Finalization is Destruction idiom_
(RFID). Mais généralement on se contente de l'appeler
[_Resource Acquisition is Initialization idiom_](http://cpp.developpez.com/faq/cpp/?page=pointeurs#POINTEURS_raii)
(RAII) car le principe est qu'une ressource à peine est-t-elle allouée, elle
doit aussitôt être confiée à une capsule RAII qui assurera sa libération
déterministe.

Le standard C++98/03 n'offre qu'une seule capsule RAII généraliste, mais elle
est assez limitée et elle vient avec des effets de bord indésirables pour les
non-avertis. Il est toutefois facile de trouver des _scoped guards_ prêts à
l'emploi, à commencer par chez [boost](http://www.boost.org/). Toutes les
collections standards suivent le principe du RAII ; ce qui explique pourquoi le
type `std::string` est si vite adopté par les développeurs, et pourquoi on
cherche à orienter vers des `std::vector<>` pour gérer des tableaux. Le dernier
standard paru en 2011 introduit enfin des scoped guards standards et sains, et
des types dans la continuité du RAII : des pointeurs dit intelligents.

Ainsi, si nous reprenons l'exemple de la section précédente, le code devient une fois corrigé :

```cpp
    shared_ptr<NotifyIcon> CreateNotifyIcon()
    {
        shared_ptr<NotifyIcon> icon(new NotifyIcon());
        icon->set_text("Blah blah blah");
        shared_ptr<Icon> inner( new Icon(...) );
        icon->set_icon(inner, GetInfo());
        icon->set_visible(true);
        return icon;
    }
```

Ou en version C++14 :

```cpp
    // Ou en version C++14
    unique_ptr<NotifyIcon> CreateNotifyIcon()
    {
        auto icon {make_unique<NotifyIcon>()};
        icon->set_text("Blah blah blah");
        auto inner {make_unique<Icon>(...)};
        icon->set_icon(move(inner), GetInfo());
        icon->set_visible(true);
        return icon;
    }
```

Peu importe si les fonctions appelées échouent, peu importe si elles viennent à
être réordonnées, nous avons la certitude que `inner` sera libérée (ou confié à
`icon`), et que `icon` sera libérée en cas de soucis, ou retournée dans le cas
nominal.

Il est intéressant de noter que le RAII est applicable non seulement avec un
code construit avec des exceptions, mais aussi avec un code continuant à
fonctionner avec des codes de retour pour assurer la propagation des erreurs.

À vrai dire bien qu'il s'agisse d'une spécificité du C++, les autres langages
pourvus d'exceptions disposent généralement d'un équivalent avec le
[_dispose-pattern_](http://en.wikipedia.org/wiki/Dispose_pattern)
(`try`-`catch`-`finally`) qui permet d'obtenir le même comportement
mais de façon explicite et non plus implicite. Si en plus ce langage est pourvu
d'un garbage collector, la gestion de la mémoire est encore gérée autrement
alors que le C++ nous oriente vers une solution unique pour gérer toutes les
ressources, sans distinctions. Il est aussi à noter que C# fut le premier des
descendants mainstream du C++ à introduire une alternative implicite et
déterministe au dispose-pattern via le mot clé `using`, et Java s'y est également
mis avec l'introduction des
[_try-with-resources_](http://docs.oracle.com/javase/tutorial/essential/exceptions/tryResourceClose.html)
dans sa version 7.

NB : Pour certains, « C++ moderne » pourrait rimer avec méta-programmation
template et autres joyeusetés très puissantes et vite arcaniques qui sont au
cœur du projet qui sert de laboratoire aux évolutions de la bibliothèque
standard : boost. Certes, c'est une utilisation moderne du langage, d'une
certaine façon, mais ce n'est pas la modernité que l'on attend du simple
développeur lambda d'applications. Il est attendu de lui qu'il puisse écrire
simplement du code qui réponde aux besoins ; la robustesse et la maintenabilité
étant deux des besoins implicites. Suivre l'«approche moderne» décrite
précédemment est un premier pas dans cette direction.

Le C++ moderne, c'est aussi la bibliothèque standard, qui non seulement est
dans la continuité du RAII, mais qui aussi fournit des outils génériques à des
besoins récurrents (collections, algorithmes, chaînes de caractères
simplifiées) et pas seulement ces flux rigolos avec des `<<` et des `>>`.

Le C++ moderne, c'est aussi l'application d'idiomes (/patterns) modernisés. Par
exemple, exit le test pour prévenir l'auto-affectation qui ne garantit pas
l'_exception-safety_, mais bonjour l'idiome
[_copy-and-swap_](http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Copy-and-swap).
Le C++ moderne c'est une nouvelle façon de penser en C++ qui implique une
nouvelle façon d'enseigner le C++.

Malgré cela, le C++ reste complexe sur bien des points très techniques (comment
changer son allocateur, comment écrire des méta-programmes template, etc.) en
plus des points hérités du C. Il introduit aussi la complexité de la
modélisation objet, à commencer par le Principe de [_Substitution de Liskov_](http://cpp.developpez.com/faq/cpp/?page=heritage#heritage_lsp) (LSP)
qui est une pierre angulaire pour savoir quand on peut hériter, ou encore la
[_Loi de Déméter_](http://blog.emmanueldeloget.com/index.php?post/2007/02/15/50-la-loi-de-demeter)
qui cherche à nous enseigner la différence entre faire soit même et déléguer.
Il introduit aussi des choses assez spécifiques comme la distinction entre la
[_sémantique de valeur_](http://cpp.developpez.com/faq/cpp/?page=classes#CLASS_valeur) et la
[_sémantique d'entité_](http://cpp.developpez.com/faq/cpp/?page=classes#CLASS_entite) à
cause de sa dualité quant aux accès directs ou indirects aux objets.

Et à aucun moment le C++98/03 n'adresse la question de la programmation
concurrente ou parallèle.

### Addendum post C++11 (EDIT de mars 2014)

L'arrivée des compilateurs C++11, voire C++14 peut jouer sur la définition de
_moderne_ dans le cadre du C++. Jusqu'à lors, la distinction
_moderne_/_historique_ se limitait à C++ à la C VS C++ 98/03 avec bonnes
pratiques dont le RAII. Le C++11 entérine les pointeurs intelligents, mais il
apporte aussi son lot d'autres simplifications comme `auto`, les
_range-based for loops_, ou de fonctionnalités comme les lambdas.

Mon appréciation de la pratique du C++ à la sortie de l'école, et en industrie,
est telle que même à l'orée du C++14, je continue à employer _moderne_ dans le
sens de _avec RAII_, et pas encore dans le sens: C++11/14 en opposition au
C++98/03 avec les bonnes pratiques associées.

## Conclusion

Votre serviteur a profité de sa soutenance N3 _[NdA: Les «experts» soutiennent
sur un sujet en rapport avec leur domaine dans ma boite pour faire reconnaitre
leur status.]_ pour reprendre un chapitre du mémoire qui répondait à la
question «C'est quoi le C++ moderne ?». J'espère avoir répondu à la question,
mais surtout de vous avoir convaincu de la nécessité de cesser de surveiller
tous les chemins possibles dans un code pour à la place surveiller toutes les
ressources manipulées.
