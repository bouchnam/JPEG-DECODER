# Projet C - décodeur JPEG



### Compilation

Pour compiler les fichiers, il suffit de taper la commande **make** dans la racine
du répertoire.


### Exécution et Tests

Après compilation, vous pouvez tester les images avec la commande **./jpeg2ppm Nom_de_l'image**

Pour tester le décodeur, commencez par tester les images en niveux de gris :

invader.jpeg   : Image 8x8  
gris.jpg       : Plusieurs blocs  
bisou.jpeg     : Troncature à droite et en bas


Testez ensuite les images en couleur :  

zig-zag.jpg           : Sans troncature  
thumbs.jpg            : Tronquée à droite et en bas  
horizontal.jpg        : Echantillonnage horizontal  
vertical.jpg          : Echantillonnage vertical  
shaun_the_sheep.jpeg  : Echantillonnage horizontal et vertical
