/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

/**
 * @file io/viewers/viewer3D-9-3Dimages.cpp
 * @ingroup Examples
 * @author Bertrand Kerautret (\c kerautre@loria.fr )
 * LORIA (CNRS, UMR 7503), University of Nancy, France
 *
 * @date 2013/05/19
 *
 * An example file named viewer3D-8-2Dimages.
 *
 * This file is part of the DGtal library.
 */



/**
 * \image html  viewer3D-9.png  "Illustration of the 3D image visualization."
 *  \example io/viewers/viewer3D-9-3Dimages.cpp
 *
 */

/*
 * TP1 Prise en main DGtal de Ricardo Uribe Lobello
 * Fait par : MATTIOLI Pierre & MABILY Johan
 */

///////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "DGtal/base/Common.h"
#include "DGtal/io/readers/GenericReader.h"
#include "DGtal/images/ImageHelper.h"
#include "DGtal/images/Image.h"
#include "ConfigExamples.h"

#include "DGtal/helpers/StdDefs.h"

#include "DGtal/io/viewers/Viewer3D.h"
#include "DGtal/io/DrawWithDisplay3DModifier.h"
#include "DGtal/io/colormaps/HueShadeColorMap.h"
#include "DGtal/io/Color.h"

#include "DGtal/kernel/SpaceND.h"
#include "DGtal/kernel/domains/HyperRectDomain.h"
#include "DGtal/images/ImageSelector.h"

#include "DGtal/geometry/volumes/distance/DistanceTransformation.h"
#include "DGtal/images/SimpleThresholdForegroundPredicate.h"
#include "DGtal/helpers/StdDefs.h"

///////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace DGtal;



struct hueFct{
 inline
 unsigned int operator() (unsigned int aVal) const
  {
    HueShadeColorMap<unsigned int>  hueShade(0,255);
    Color col = hueShade((unsigned int)aVal);
    return  (((unsigned int) col.red()) <<  16)| (((unsigned int) col.green()) << 8)|((unsigned int) col.blue());
  }
};

// Définition du type de viewer à utiliser. 
typedef Viewer3D<> ViewerType ;
// Définition du type de conteneur à utiliser pour l'image du premier exercice. 
typedef ImageContainerBySTLVector<Z3i::Domain,  float> Image3D;
// Définition du type de conteneur à utiliser pour l'image de la transformée en distance. 
typedef ImageSelector<Z3i::Domain, unsigned char>::Type Image;

// Parcours et traitement d'un volume. 
void imageSandbox(ViewerType& viewer, std::string filename);
// Tranformée en distance.
void transformeeEnDistance(ViewerType& viewer, std::string filename);

// Méthode pour générer des voxels de manière aléatoire. 
template<typename Image>
void randomSeeds(Image &image, const unsigned int nb, const int value);

///////////////////////////////////////////////////////////////////////////////

int main( int argc, char** argv )
{
  QApplication application(argc,argv);
  ViewerType viewer;
  
  // Appel aux méthodes des exercices.
  if (argc > 2)
  {
    std::stringstream ssAlgo;
    ssAlgo << std::string(argv[1]);

    std::stringstream ssFile;
    ssFile << std::string(argv[2]);
    if (ssAlgo.str() == "Sandbox")
      imageSandbox(viewer, ssFile.str());
    else if (ssAlgo.str() == "DT")
      transformeeEnDistance(viewer, ssFile.str());
  }
  else
  {
    std::cout << "Les paramètres n'a pas été fourni !!!!!!!." << std::endl;
    std::cout << "startCode <paramètre = {Sandbox | DT}> <nom du fichier>" << std::endl;
    return 0;
  }
  
  return application.exec();
}


/**
 * Cette fonction vous permettra de commencer à pratiquer avec le chargement d'objets 
 * volumiques. Les parcourir, retrouver les valeurs affectées à chaque voxel et les 
 * les modifier. 
 * \param Visualisateur à utiliser.
 * \param Nom du fichier.
 *
 */
void imageSandbox(ViewerType& viewer, std::string filename)
{
  // Lance le visusalisateur. 
  viewer.show();
  
  //Chargement d'une image dans une structure de données ImageContainerBySTLVector.
  std::string inputFilename = examplesPath + "/" + filename;
  Image3D image = GenericReader<Image3D>::import(inputFilename);

  // *** Initialisation des variables *** //
  
  // Obtention du domaine (taille) de l'image chargée. 
  Z3i::Domain initialDomain = image.domain();

  // Définition du gradient des couleurs Question A-5
  GradientColorMap<long> gradient(0,30);
  gradient.addColor(Color::Red);
  gradient.addColor(Color::Yellow);
  gradient.addColor(Color::Green);
  gradient.addColor(Color::Cyan);
  gradient.addColor(Color::Blue);
  gradient.addColor(Color::Magenta);
  gradient.addColor(Color::Red);
  
  //min et max pour A-2
  float min = 100.0; //min -> 100
  float max = 150.0; //max -> 150
  
  // Sous-domaine
  Z3i::Point pointInitial (0.0,0.0,0.0);
  Z3i::Point pointFinal (50.0,50.0,50.0);
  Z3i::Domain sousDomain (pointInitial, pointFinal);
  
  
  // *** Questions *** //
  
  //A-1) Apres avoir analyse le code, nous voyons que tous les voxels > 0 de notre volume sont affichés (viewer) en sortie a l'aide de DGtal.
  //A-2) En testant image(*it) qui représente la valeur d'un voxel, on choisit min 100 et max 150 pour les voxels sur lobster et on obtient un homard en sortie.
  //A-3) Pour tester avec les coordonnees x, utiliser z3i::point pts = *it puis if pts[0] < 50, et faire le viewer du volume (pts[0] etant x). On observe seulement une partie de notre volume.
  //A-4) Pour tester, faire un iterateur parcourant le sous domaine cree et faire le viewer des voxels.
  //A-5) On additionne toutes les coordonnees x,y,z entre elles et on divise par le nombre de voxels pour obtenir les coordonnees du centre c.
  //     On calcule ensuite la distance euclidienne au carre par rapport au centre c pour le voxel courant.
  //     Et enfin, on colorie le voxel courant en fonction de la distance obtenue.
  
  
  /* ************************************  
     Decommentez les codes des tests selon 
     votre souhait pour executer le 
     code de la question choisie.
     Le test A-5 est decommente de base.
     ************************************ */
  
  
  // *** test A-1 *** //
  
  /*for(Z3i::Domain::ConstIterator it= image.domain().begin(),
	itend = image.domain().end(); it != itend; ++it)
  {
    if (image(*it) > 0)
      viewer << *it;
  }*/
  
  
  // *** test A-2 *** //
  
  /*for(Z3i::Domain::ConstIterator it= image.domain().begin(),
	itend = image.domain().end(); it != itend; ++it)
  {
    if ((image(*it) >=100) && (image(*it) <= 150))
      viewer << *it;
  }*/
  
  
  // *** test A-3 *** //
  
  /*for(Z3i::Domain::ConstIterator it= image.domain().begin(),
	itend = image.domain().end(); it != itend; ++it)
  {
    Z3i::Point pts = *it;
    if ((pts[0]<=50) && (image(*it)>0))//si la coordonnees x < 50
      viewer << *it;
  }*/
  
  
  // *** test A-4 *** //
  
  /*for(Z3i::Domain::ConstIterator it= sousDomain.begin(),
    itend = sousDomain.end(); it != itend; ++it)
  {
      if (image(*it) > 0)
        viewer << *it;
  }*/
  
  
  // *** test A-5 *** //
  
  //calcul du centre
  
  float c_x=0,c_y=0,c_z=0;
  float nb_voxels=0.0;
  //Z3i::Point c = *it;
  
  //on additionne toutes les coordonnees x,y,z entre elles et on divise par le nombre de voxels pour obtenir les coordonnees du centre c.
  for(Z3i::Domain::ConstIterator it= image.domain().begin(),
	itend = image.domain().end(); it != itend; ++it)
  {
      Z3i::Point pts = *it;
      if (image(*it) > 0){
        c_x+=pts[0];
        c_y+=pts[1];
        c_z+=pts[2];
        nb_voxels++;
      }
  }
  //centre du volume c
  Z3i::Point c(c_x/nb_voxels,c_y/nb_voxels,c_z/nb_voxels);
  std::cout<<"centre c : ("<< c[0] << "," << c[1] << "," << c[2] << ")" << std::endl;
  
  //distance euclidienne au carre
  double distE2;
  
  for(Z3i::Domain::ConstIterator it= image.domain().begin(),
	itend = image.domain().end(); it != itend; ++it)
  {
      Z3i::Point pts = *it;
      if (image(*it) > 0){
          
        // Calcul de la distance euclidienne au carre par rapport au centre pour le voxel courant 
        distE2= ((pts[0]-c[0])*(pts[0]-c[0])+(pts[1]-c[1])*(pts[1]-c[1])+(pts[2]-c[2])*(pts[2]-c[2]));
    
        // Calcul du gradient de couleur pour cette distance.
        Color coul= gradient(distE2);
        viewer << CustomColors3D(Color((float)(coul.red()),
				   (float)(coul.green()),
				   (float)(coul.blue(),205)),
                    Color((float)(coul.red()),
				   (float)(coul.green()),
				   (float)(coul.blue()),205));
        viewer << *it;
      }
  }
  
  
  viewer << SetMode3D(image.className(), "BoundingBox");
  viewer << ViewerType::updateDisplay;
}

/**
 * Fonction de la transformée en distance à partir de quelques points germes.
 * La distance est calculée à partir de chaque point. Donc, la distance dans un 
 * voxel est la distance minimale à tous les points germes. 
 * \param le visualisateur à utiliser. 
 * \param Nom du fichier.
 *
 */
void transformeeEnDistance(ViewerType& viewer, std::string filename)
{
  // Affichage de la visualisation. 
  viewer.show();
  // Nombre du fichier à charger. 
  std::string inputFilename = examplesPath + "/" + filename;

  // Création du type d'image. 
  //Default image selector = STLVector
  typedef ImageSelector<Z3i::Domain, unsigned char>::Type Image;

  //Chargement du fichier image dans la structure. 
  Image image = VolReader<Image>::importVol( inputFilename );
  // Obtention du domaine (taille) de l'image. 
  Z3i::Domain domain = image.domain();
  

  // Création d'une image pour générer les germes. 
  Image imageSeeds ( domain);
  for ( Image::Iterator it = imageSeeds.begin(), itend = imageSeeds.end();it != itend; ++it)
    (*it)=1;

  // Génération aléatoire des germes. 
  randomSeeds(imageSeeds, 70, 0);
  
  //Preparation du prédicat de la transformée. Un simple prédicat de seuil. 
  typedef functors::SimpleThresholdForegroundPredicate<Image> Predicate;
  Predicate aPredicate(imageSeeds,0);

  // Création de type et de l'objet pour appliquer la transformée. 
  typedef  DistanceTransformation<Z3i::Space,Predicate, Z3i::L2Metric> DTL2;
  DTL2 dtL2(&domain, &aPredicate, &Z3i::l2Metric);

  // Detection des distances minimales et maximales. 
  unsigned int min = 0;
  unsigned int max = 0;
  for(DTL2::ConstRange::ConstIterator it = dtL2.constRange().begin(),
	itend=dtL2.constRange().end();
      it!=itend;
      ++it)
  {
    if(  (*it) < min )
      min=(*it);
    if( (*it) > max )
      max=(*it);
  }
  
  //Spécification des gradients de couleur pour la visualisation.
  GradientColorMap<long> gradient( 0,30);
  gradient.addColor(Color::Red);
  gradient.addColor(Color::Yellow);
  gradient.addColor(Color::Green);
  gradient.addColor(Color::Cyan);
  gradient.addColor(Color::Blue);
  gradient.addColor(Color::Magenta);
  gradient.addColor(Color::Red);
  

    // *** Questions *** //
  
  //B-1) Oui, c'est logique. Apres avoir analyse le code, nous retrouvons bien la transformee de distance que nous avons appris.
  //B-2) Le programme calcule le minimum de chaque voxel par rapport aux germes.
  //B-3) On reprend l'algo de notre partie A-3. On observe seulement une partie de notre volume.
  //B-4) On observe notre volume sans les iso-surfaces discretes de la fonction definie par la transformee en distance.
  //B-5) Il suffit d'inverser nos calculs sur le complementaire de la forme et extraire la transformee en distance 3D.
  
  
  // *** test B-1 & B-2 & B-4 *** //
  
  
  // Affectation du mode de visualisation 3D. 
  viewer << SetMode3D( (*(domain.begin())).className(), "Paving" );
  // Parcours de tous les voxels de l'image avec un iterateur sur le domaine.  
  for(Z3i::Domain::ConstIterator it = domain.begin(), itend=domain.end();
      it!=itend;
      ++it)
  {
    // Calcul de la transformée en distance pour le voxel courant. 
    double valDist= dtL2( (*it) );
    
    // Calcul du gradient de couleur pour cette distance.
    Color c= gradient(valDist);
    viewer << CustomColors3D(Color((float)(c.red()),
				   (float)(c.green()),
				   (float)(c.blue(),205)),
			     Color((float)(c.red()),
				   (float)(c.green()),
				   (float)(c.blue()),205));
    // Le viewer reçoit le prochain voxel pour visualisation.
    if (image(*it) > 0)
        viewer << *it ;
  }
  
  
  // *** test B-3 *** //
  
  /*
  for(Z3i::Domain::ConstIterator it = domain.begin(), itend=domain.end();
      it!=itend;
      ++it)
  {
    // Calcul de la transformée en distance pour le voxel courant. 
    double valDist= dtL2( (*it) );
    
    // Calcul du gradient de couleur pour cette distance.
    Color c= gradient(valDist);
    viewer << CustomColors3D(Color((float)(c.red()),
				   (float)(c.green()),
				   (float)(c.blue(),205)),
			     Color((float)(c.red()),
				   (float)(c.green()),
				   (float)(c.blue()),205));
    // Le viewer reçoit le prochain voxel pour visualisation.
    Z3i::Point pts = *it;
    if ((pts[0]<=50) && (image(*it)>0)) //si la coordonnees x < 50
      viewer << *it;
  }
  */
  
  
  //viewer << ClippingPlane(0,1,0, -40) << Viewer3D<>::updateDisplay;
  // Mise à jour du visualisateur après le parcours de tous le voxels. 
  viewer<< Viewer3D<>::updateDisplay;
}


/**
 * Cette fonction genère un ensemble de points afin de les placer 
 * dans le volume comme les germes de la transformée en distance. 
 * \param image.
 * \param nombre de germes.
 * \param value à affecter comme seuil. 
 *
 */ 
template<typename Image>
 void randomSeeds(Image &image, const unsigned int nb, const int value)
 {
   typename Image::Point p, low = image.domain().lowerBound();
   typename Image::Vector ext;
   srand ( time(NULL) );
 
   ext = image.extent();
 
   for (unsigned int k = 0 ; k < nb; k++)
     {
       for (unsigned int dim = 0; dim < Image::dimension; dim++)
         p[dim] = rand() % (ext[dim]) +  low[dim];
 
       image.setValue(p, value);
     }
 }
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
