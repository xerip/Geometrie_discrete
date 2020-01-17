/*
    Exemples de transformations en OpenCV, avec zoom, seuil et affichage en
    couleurs. L'image de niveau est en CV_32SC1.

    g++ -Wall --std=c++14 $(pkg-config opencv --cflags)  ex01-transfos.cpp \
                          $(pkg-config opencv --libs) -o ex01-transfos
    ./ex01-transfos [-mag width height] [-thr seuil] image_in [image_out]

    CC-BY Edouard.Thiel@univ-amu.fr - 30/09/2019
     
    MATTIOLI PIERRE Version 1 
*/

#include <iostream>
#include <vector>
#include <cstring>
#include <opencv2/opencv.hpp>

#define CHECK_MAT_TYPE(mat, format_type) \
    if (mat.type() != int(format_type)) \
        throw std::runtime_error(std::string(__func__) +\
            ": format non géré '" + std::to_string(mat.type()) +\
            "' pour la matrice '" # mat "'");


//--------------------------------- L O U P E ---------------------------------

class Loupe {
  public:
    int zoom = 5;
    int zoom_max = 20;
    int zoom_x0 = 0;
    int zoom_y0 = 0;
    int zoom_x1 = 100;
    int zoom_y1 = 100;

    void reborner (cv::Mat &res1, cv::Mat &res2)
    {
        int bon_zoom = zoom >= 1 ? zoom : 1;

        int h = res2.rows / bon_zoom;
        int w = res2.cols / bon_zoom;

        if (zoom_x0 < 0) zoom_x0 = 0;
        zoom_x1 = zoom_x0 + w;
        if (zoom_x1 > res1.cols) {
            zoom_x1 = res1.cols;
            zoom_x0 = zoom_x1 - w;
            if (zoom_x0 < 0) zoom_x0 = 0;
        }

        if (zoom_y0 < 0) zoom_y0 = 0;
        zoom_y1 = zoom_y0 + h;
        if (zoom_y1 > res1.rows) {
            zoom_y1 = res1.rows;
            zoom_y0 = zoom_y1 - h;
            if (zoom_y0 < 0) zoom_y0 = 0;
        }
    }

    void deplacer (cv::Mat &res1, cv::Mat &res2, int dx, int dy)
    {
        zoom_x0 += dx; zoom_y0 += dy; 
        zoom_x1 += dx; zoom_y1 += dy; 
        reborner (res1, res2);
    }

    void dessiner_rect (cv::Mat &src, cv::Mat &dest)
    {
        dest = src.clone();
        if (zoom == 0) return;
        cv::Point p0 = cv::Point(zoom_x0, zoom_y0),
                  p1 = cv::Point(zoom_x1, zoom_y1);
        cv::rectangle(dest, p0, p1, cv::Scalar (255, 255, 255), 3, 4);
        cv::rectangle(dest, p0, p1, cv::Scalar (  0,   0, 255), 1, 4);
    }

    void dessiner_portion (cv::Mat &src, cv::Mat &dest)
    {
        CHECK_MAT_TYPE(src, CV_8UC3)

        int bon_zoom = zoom >= 1 ? zoom : 1;

        for (int y = 0; y < dest.rows; y++)
        for (int x = 0; x < dest.cols; x++)
        {
            int x0 = zoom_x0 + x / bon_zoom;
            int y0 = zoom_y0 + y / bon_zoom;

            if (x0 < 0 || x0 >= src.cols || y0 < 0 || y0 >= src.rows) {
                dest.at<cv::Vec3b>(y,x)[0] = 64;
                dest.at<cv::Vec3b>(y,x)[1] = 64;
                dest.at<cv::Vec3b>(y,x)[2] = 64;
                continue;
            }
            dest.at<cv::Vec3b>(y,x)[0] = src.at<cv::Vec3b>(y0,x0)[0];
            dest.at<cv::Vec3b>(y,x)[1] = src.at<cv::Vec3b>(y0,x0)[1];
            dest.at<cv::Vec3b>(y,x)[2] = src.at<cv::Vec3b>(y0,x0)[2];
        }
    }
};


//----------------------- C O U L E U R S   V G A -----------------------------

void representer_en_couleurs_vga (cv::Mat img_niv, cv::Mat img_coul)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)
    CHECK_MAT_TYPE(img_coul, CV_8UC3)

    unsigned char couls[16][3] = {  // R, G, B
        {   0,   0,   0 },   //  0  black           ->  0 uniquement
        {  20,  20, 190 },   //  1  blue            ->  1, 15, 29, ...
        {  30, 200,  30 },   //  2  green           ->  2, 16, 30, ...
        {  30, 200, 200 },   //  3  cyan            ->  3, 17, 31, ...
        { 200,  30,  30 },   //  4  red             ->  4, 18, 32, ...
        { 200,  30, 200 },   //  5  magenta         ->  5, 19, 33, ...
        { 200, 130,  50 },   //  6  brown           ->  6, 20, 34, ...
        { 200, 200, 200 },   //  7  light gray      ->  7, 21, 35, ...
        { 110, 110, 140 },   //  8  dark gray       ->  8, 22, 36, ...
        {  84, 130, 252 },   //  9  light blue      ->  9, 23, 37, ...
        {  84, 252,  84 },   // 10  light green     -> 10, 24, 38, ...
        {  84, 252, 252 },   // 11  light cyan      -> 11, 25, 39, ...
        { 252,  84,  84 },   // 12  light red       -> 12, 26, 40, ...
        { 252,  84, 252 },   // 13  light magenta   -> 13, 27, 41, ...
        { 252, 252,  84 },   // 14  yellow          -> 14, 28, 42, ...
        { 252, 252, 252 },   // 15  white           -> 255 uniquement
    };

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x), c = 0;
        if (g == 255) c = 15;                      // seul 255 est blanc
        else if (g != 0) c = 1 + abs(g-1) % 14;    // seul 0 est noir
        // Attention img_coul est en B, G, R -> inverser les canaux
        img_coul.at<cv::Vec3b>(y,x)[0] = couls[c][2];
        img_coul.at<cv::Vec3b>(y,x)[1] = couls[c][1];
        img_coul.at<cv::Vec3b>(y,x)[2] = couls[c][0];
    }
}


//----------------------------------- M Y -------------------------------------

class My {
  public:
    cv::Mat img_src, img_res1, img_res2, img_niv, img_coul;
    Loupe loupe;
    int seuil = 127;
    int clic_x = 0;
    int clic_y = 0;
    int clic_n = 0;

    enum Recalc { R_RIEN, R_LOUPE, R_TRANSFOS, R_SEUIL };
    Recalc recalc = R_SEUIL;

    void reset_recalc ()             { recalc = R_RIEN; }
    void set_recalc   (Recalc level) { if (level > recalc) recalc = level; }
    int  need_recalc  (Recalc level) { return level <= recalc; }

    // Rajoutez ici des codes A_TRANSx pour le calcul et l'affichage
    enum Affi { A_ORIG, A_SEUIL, A_TRANS1, A_TRANS2, A_TRANS3, A_TRANS4 };
    Affi affi = A_ORIG;
};


//----------------------- T R A N S F O R M A T I O N S -----------------------

void inverser_couleurs (cv::Mat img)
{
    CHECK_MAT_TYPE(img, CV_8UC3)

    for (int y = 0; y < img.rows; y++)
    for (int x = 0; x < img.cols; x++)
    {
        img.at<cv::Vec3b>(y,x)[0] = 255 - img.at<cv::Vec3b>(y,x)[0];
        img.at<cv::Vec3b>(y,x)[1] = 255 - img.at<cv::Vec3b>(y,x)[1];
        img.at<cv::Vec3b>(y,x)[2] = 255 - img.at<cv::Vec3b>(y,x)[2];
    }
}


// Placez ici vos fonctions de transformations à la place de ces exemples

// ****************************** TP1 ****************************** A FINIRRRRR !

void marquer_contours_c8 (cv::Mat img_niv) // exercice a
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows-1; y++)
    {
    for (int x = 0; x < img_niv.cols-1; x++)
    {
        int g = img_niv.at<int>(y,x);
        //if (g > 0) {
        //    img_niv.at<int>(y,x) = y;
        if (g!=0)
        {
			if (x==0 || x==img_niv.cols-1 // on verifie chaque pixel du 4-voisinage si celui-ci est un point contour
				|| y==0 || y==img_niv.rows-1 
				|| img_niv.at<int>(y,x-1)==0 
				|| img_niv.at<int>(y,x+1)==0
				|| img_niv.at<int>(y-1,x)==0 
				|| img_niv.at<int>(y+1,x)==0)
				img_niv.at<int>(y,x) = 1; //on lui met la label 1 si c'est un point contour
		}
        }
    }
}


void marquer_contours_c4 (cv::Mat img_niv) //exercice b
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        //if (g > 0) {
        //   img_niv.at<int>(y,x) = x;
        if (g!=0)
        {
			if (x==0 || x==img_niv.cols-1 //on verifie chaque pixel du 8-voisinage si celui-ci est un point contour
				|| y==0 || y==img_niv.rows-1
				|| img_niv.at<int>(y-1,x-1)==0 
				|| img_niv.at<int>(y,x-1)==0
				|| img_niv.at<int>(y+1,x-1)==0 
				|| img_niv.at<int>(y,x+1)==0
				|| img_niv.at<int>(y-1,x)==0 
				|| img_niv.at<int>(y-1,x+1)==0
				|| img_niv.at<int>(y+1,x+1)==0  
				|| img_niv.at<int>(y+1,x)==0)
				img_niv.at<int>(y,x) = 1; //on lui met la label 1 si c'est un point contour
		}
    }
}


void numeroter_contours_c8 (cv::Mat img_niv) //exercice final
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)
    
    //int label=1; //variable pour l'etape 1 seule (sans groupe d'equivalence)
    int label_max=1;
    std::vector<int> p8v(8,0); //liste des pixels appartenant au 8-voisinage
    std::vector<int> label_dispo; //liste des labels disponibles pour chaque pixel
    std::vector<std::vector<int>> tab_eq; //groupe d'equivalence
    
    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g!=0)
        {
			if (x==0 || x==img_niv.cols-1 // si le pixel est un point contour
				|| y==0 || y==img_niv.rows-1 
				|| img_niv.at<int>(y,x-1)==0 
				|| img_niv.at<int>(y,x+1)==0
				|| img_niv.at<int>(y-1,x)==0 
				|| img_niv.at<int>(y+1,x)==0)
                {
                //etape 1 seule (sans groupe d'equivalence)
                    
                /*if (img_niv.at<int>(y-1,x-1)!=0 && img_niv.at<int>(y-1,x-1)!=255)
                    img_niv.at<int>(y,x)=img_niv.at<int>(y-1,x-1);
                else if (img_niv.at<int>(y-1,x)!=0 && img_niv.at<int>(y-1,x)!=255)
                    img_niv.at<int>(y,x)=img_niv.at<int>(y-1,x);
                else if (img_niv.at<int>(y-1,x+1)!=0 && img_niv.at<int>(y-1,x+1)!=255)
                    img_niv.at<int>(y,x)=img_niv.at<int>(y-1,x+1);
                else if (img_niv.at<int>(y,x-1)!=0 && img_niv.at<int>(y,x-1)!=255)
                    img_niv.at<int>(y,x)=img_niv.at<int>(y,x-1);
                else if (img_niv.at<int>(y,x+1)!=0 && img_niv.at<int>(y,x+1)!=255)
                    img_niv.at<int>(y,x)=img_niv.at<int>(y,x+1);
                else if (img_niv.at<int>(y+1,x-1)!=0 && img_niv.at<int>(y+1,x-1)!=255)
                    img_niv.at<int>(y,x)=img_niv.at<int>(y+1,x-1);
                else if (img_niv.at<int>(y+1,x)!=0 && img_niv.at<int>(y+1,x)!=255)
                    img_niv.at<int>(y,x)=img_niv.at<int>(y+1,x);
                else if (img_niv.at<int>(y+1,x+1)!=0 && img_niv.at<int>(y+1,x+1)!=255)
                    img_niv.at<int>(y,x)=img_niv.at<int>(y+1,x+1);
                else {
                    img_niv.at<int>(y,x)=label;
                    label++;
                    if (label==255) label++;}
                }*/
                
                // etape 1 + groupe d'equivalence => exercice final
                
                //p8v contient la valeur des pixels appartenant au 8-voisinage du pixel ou l'on se trouve
                p8v[0]=img_niv.at<int>(y,x-1);
                p8v[1]=img_niv.at<int>(y-1,x-1);
                p8v[2]=img_niv.at<int>(y-1,x);
                p8v[3]=img_niv.at<int>(y-1,x+1);
                p8v[4]=img_niv.at<int>(y,x+1);
                p8v[5]=img_niv.at<int>(y+1,x-1);
                p8v[6]=img_niv.at<int>(y+1,x);
                p8v[7]=img_niv.at<int>(y+1,x+1);
                for (int i=0; i<8; i++)
                    {if ((p8v[i]!=0) && (p8v[i]!=255)) label_dispo.push_back(p8v[i]);}
                if (label_dispo.size()==0) //si aucun label n'est disponible
                    {
                        img_niv.at<int>(y,x)=label_max; //le pixel prend la valeur d'un nouveau label
                        label_max++;
                        if (label_max==255) label_max++; //on ignore la valeur 255 qui est la valeur pour la forme dans l'image
                    }
                else if (label_dispo.size()==1) //si un seul label est disponible
                    {
                        img_niv.at<int>(y,x)=label_dispo[0]; //le pixel prend la valeur de ce label
                        label_dispo.pop_back();
                    }
                else //si plusieurs labels sont disponibles 
                    {   
                        tab_eq.push_back(label_dispo); //on remplit notre table d'equivalence
                        //for (int j=0; j<label_dispo.size(); j++)
                        //    std::cout << label_dispo[j] << "  ";
                        //std::cout << std::endl;
                        img_niv.at<int>(y,x)=label_dispo[0]; //le pixel prend la valeur d'un des labels disponibles
                        while (!label_dispo.empty()) label_dispo.pop_back(); //on vide notre liste de label disponible pour le prochain pixel
                    }
                }
        }
    }
}

// ****************************** TP2 ******************************

//structure TP3
struct ContourF8 {
    int xA;
    int yA;
    std::vector<int> cdf;
    int t_cdf;
};

struct ContourPol {
    //std::vector<(int,int)> coords;
    std::vector<int> cx;
    std::vector<int> cy;
    std::vector<int> flag;
    int t_c;
};

void suivre_un_contour_c8 (cv::Mat img_niv, int xA, int yA, int dirA, int num_contour, std::vector<ContourF8> memo_contour)
{
    // 1) recherche de la direction d'arrivee sur xA,yA :
    //    on tourne autour de xA,yA dans le sens croissant à partir de dirA
    int nx8[]={1,1,0,-1,-1,-1,0,1};
    int ny8[]={0,1,1,1,0,-1,-1,-1};
    int d=0, dir_finale=0; 
    
    // TP3
    //int taille=0;
    std::vector<int> cdf;
    //ContourF8 cF8 {xA, yA, cdf, taille};
    
    for (int i=0; i<8; i++){
        d = (dirA+i) % 8;
        // si N8(A,d) est dans l'image et > 0:
        if (img_niv.at<int>(yA+ny8[d],xA+nx8[d])>0)
        {
            dir_finale=(d+4)%8;
            break;
        }
    }
    // 2) suivi et marquage du contour
    int x = xA, y = yA, dir = dir_finale;
    do 
    {
        img_niv.at<int>(yA,xA)=num_contour;
        // direction de départ pour chercher le point suivant
        dir = (dir+4-1) % 8;
        // on tourne autour de x,y dans le sens décroissant à partir de dir
        for (int i=0; i<8; i++)
        {
            d = (dir+8-i) % 8;
            //Q = N8((x,y),d)
            if (img_niv.at<int>(y+ny8[d],x+nx8[d])>0) 
            { 
                y = y+ny8[d];
                x = x+nx8[d]; 
                dir = d;
                // emplacement pour mémoriser d dans la chaîne de Freeman
                cdf.push_back(d);
                break;
            }
            if (i==8) break; // aucun point trouvé -> point isolé
        }
    } while (!(x==xA && y==yA && dir==dir_finale));
    
    // TP3
    memo_contour.push_back({xA, yA, cdf, cdf.size()});
    
    while (!cdf.empty()) cdf.pop_back(); 
    
}


void effectuer_suivi_contours_c8(cv::Mat img_niv)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1);
    // chaine de freeman a memoriser
    std::vector<ContourF8> memo_contour;
    
    int num_contour = 1;
    int dir;
    for (int y = 0; y < img_niv.rows; y++)
        for (int x = 0; x < img_niv.cols; x++)
        {
            int g = img_niv.at<int>(y,x);
            if (g==255)
            {
                dir=-1; //pas trouve
                if (x==img_niv.cols-1 || img_niv.at<int>(x+1,y)==0) dir=0;
                else if (y==img_niv.rows-1 || img_niv.at<int>(y,x+1)==0) dir=2;
                else if (x==0 || img_niv.at<int>(x-1,y)==0) dir=4;
                else if (y==0 || img_niv.at<int>(x,y-1)==0) dir=6;
                if (dir>=0) //trouve
                {
                    suivre_un_contour_c8 (img_niv, x, y, dir, num_contour++, memo_contour);
                    if (num_contour==255) num_contour++;
                }
            }
        }
}

// ********************* TP3 ********************



// Appelez ici vos transformations selon affi
void effectuer_transformations (My::Affi affi, cv::Mat img_niv)
{
    switch (affi) {
        case My::A_TRANS1 :
            marquer_contours_c8 (img_niv);
            break;
        case My::A_TRANS2 :
            marquer_contours_c4 (img_niv);
            break;
        case My::A_TRANS3 :
            numeroter_contours_c8 (img_niv);
            break;
        case My::A_TRANS4 :
            effectuer_suivi_contours_c8 (img_niv);
            break;
        default : ;
    }
}


//---------------------------- C A L L B A C K S ------------------------------

// Callback des sliders
void onZoomSlide (int pos, void *data)
{
    My *my = (My*) data;
    my->loupe.reborner (my->img_res1, my->img_res2);
    my->set_recalc(My::R_LOUPE);
}

void onSeuilSlide (int pos, void *data)
{
    My *my = (My*) data;
    my->set_recalc(My::R_SEUIL);
}


// Callback pour la souris
void onMouseEvent (int event, int x, int y, int flags, void *data)
{
    My *my = (My*) data;

    switch (event) {
        case cv::EVENT_LBUTTONDOWN :
            my->clic_x = x;
            my->clic_y = y;
            my->clic_n = 1;
            break;
        case cv::EVENT_MOUSEMOVE :
            // std::cout << "mouse move " << x << "," << y << std::endl;
            if (my->clic_n == 1) {
                my->loupe.deplacer (my->img_res1, my->img_res2, 
                    x - my->clic_x, y - my->clic_y);
                my->clic_x = x;
                my->clic_y = y;
                my->set_recalc(My::R_LOUPE);
            }
            break;
        case cv::EVENT_LBUTTONUP :
            my->clic_n = 0;
            break;
    }
}


void afficher_aide() {
        // Indiquez les transformations ici
        std::cout <<
            "Touches du clavier:\n"
            "   a    affiche cette aide\n"
            " hHlL   change la taille de la loupe\n"
            "   i    inverse les couleurs de src\n"
            "   o    affiche l'image src originale\n"
            "   s    affiche l'image src seuillée\n"
            "   1    contours pour c8\n"
            "   2    contours pour c4\n"
            "   3    numérotation des contours pour c8\n"
            "   4    suivi de contour pour c8\n"
            "   5    approximation polygonale + coloriage\n"
            "   6    approximation polygonale + remplissage\n"
            "   c    change la connexité pour les traitements suivants :\n"
            "   7    DT par pelage\n"
            "   8    maximums locaux\n"
            "   9    RDT par pelage\n"
            "  esc   quitte\n"
        << std::endl;
    }

// Callback "maison" pour le clavier
int onKeyPressEvent (int key, void *data)
{
    My *my = (My*) data;

    if (key < 0) return 0;        // aucune touche pressée
    key &= 255;                   // pour comparer avec un char
    if (key == 27) return -1;     // ESC pour quitter

    switch (key) {
        case 'a' :
            afficher_aide();
            break;
        case 'h' :
        case 'H' :
        case 'l' :
        case 'L' : {
            std::cout << "Taille loupe" << std::endl;
            int h = my->img_res2.rows, w = my->img_res2.cols; 
            if      (key == 'h') h = h >=  200+100 ? h-100 :  200;
            else if (key == 'H') h = h <= 2000-100 ? h+100 : 2000;
            else if (key == 'l') w = w >=  200+100 ? w-100 :  200;
            else if (key == 'L') w = w <= 2000-100 ? w+100 : 2000;
            my->img_res2 = cv::Mat(h, w, CV_8UC3);
            my->loupe.reborner(my->img_res1, my->img_res2);
            my->set_recalc(My::R_LOUPE);
          } break;
        case 'i' :
            std::cout << "Couleurs inversées" << std::endl;
            inverser_couleurs(my->img_src);
            my->set_recalc(My::R_SEUIL);
            break;
        case 'o' :
            std::cout << "Image originale" << std::endl;
            my->affi = My::A_ORIG;
            my->set_recalc(My::R_TRANSFOS);
            break;
        case 's' :
            std::cout << "Image seuillée" << std::endl;
            my->affi = My::A_SEUIL;
            my->set_recalc(My::R_SEUIL);
            break;

        // Rajoutez ici des touches pour les transformations
        case '1' :
            std::cout << "Transformation 1" << std::endl;
            my->affi = My::A_TRANS1;
            my->set_recalc(My::R_SEUIL);
            break;
        case '2' :
            std::cout << "Transformation 2" << std::endl;
            my->affi = My::A_TRANS2;
            my->set_recalc(My::R_SEUIL);
            break;
        case '3' :
            std::cout << "Transformation 3" << std::endl;
            my->affi = My::A_TRANS3;
            my->set_recalc(My::R_SEUIL);
            break;
        case '4' :
            std::cout << "Transformation 4" << std::endl;
            my->affi = My::A_TRANS4;
            my->set_recalc(My::R_SEUIL);
            break;

        default :
            //std::cout << "Touche '" << char(key) << "'" << std::endl;
            break;
    }
    return 1;
}


//---------------------------------- M A I N ----------------------------------

void afficher_usage (char *nom_prog) {
    std::cout << "Usage: " << nom_prog
              << "[-mag width height] [-thr seuil] in1 [out2]" 
              << std::endl;
}

int main (int argc, char**argv)
{
    My my;
    char *nom_in1, *nom_out2, *nom_prog = argv[0];
    int zoom_w = 600, zoom_h = 500;

    while (argc-1 > 0) {
        if (!strcmp(argv[1], "-mag")) {
            if (argc-1 < 3) { afficher_usage(nom_prog); return 1; }
            zoom_w = atoi(argv[2]);
            zoom_h = atoi(argv[3]);
            argc -= 3; argv += 3;
        } else if (!strcmp(argv[1], "-thr")) {
            if (argc-1 < 2) { afficher_usage(nom_prog); return 1; }
            my.seuil = atoi(argv[2]);
            argc -= 2; argv += 2;
        } else break;
    }
    if (argc-1 < 1 or argc-1 > 2) { afficher_usage(nom_prog); return 1; }
    nom_in1  = argv[1];
    nom_out2 = (argc-1 == 2) ? argv[2] : NULL;

    // Lecture image
    my.img_src = cv::imread (nom_in1, cv::IMREAD_COLOR);  // produit du 8UC3
    if (my.img_src.empty()) {
        std::cout << "Erreur de lecture" << std::endl;
        return 1;
    }

    // Création résultats
    my.img_res1 = cv::Mat(my.img_src.rows, my.img_src.cols, CV_8UC3);
    my.img_res2 = cv::Mat(zoom_h, zoom_w, CV_8UC3);
    my.img_niv  = cv::Mat(my.img_src.rows, my.img_src.cols, CV_32SC1);
    my.img_coul = cv::Mat(my.img_src.rows, my.img_src.cols, CV_8UC3);
    my.loupe.reborner(my.img_res1, my.img_res2);

    // Création fenêtre
    cv::namedWindow ("ImageSrc", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar ("Zoom", "ImageSrc", &my.loupe.zoom, my.loupe.zoom_max, 
        onZoomSlide, &my);
    cv::createTrackbar ("Seuil", "ImageSrc", &my.seuil, 255, 
        onSeuilSlide, &my);
    cv::setMouseCallback ("ImageSrc", onMouseEvent, &my);

    cv::namedWindow ("Loupe", cv::WINDOW_AUTOSIZE);
    afficher_aide();

    // Boucle d'événements
    for (;;) {

        if (my.need_recalc(My::R_SEUIL)) 
        {
            // std::cout << "Calcul seuil" << std::endl;
            cv::Mat img_gry;
            cv::cvtColor (my.img_src, img_gry, cv::COLOR_BGR2GRAY);
            cv::threshold (img_gry, img_gry, my.seuil, 255, cv::THRESH_BINARY);
            img_gry.convertTo (my.img_niv, CV_32SC1,1., 0.);
        }

        if (my.need_recalc(My::R_TRANSFOS))
        {
            // std::cout << "Calcul transfos" << std::endl;
            if (my.affi != My::A_ORIG) {
                effectuer_transformations (my.affi, my.img_niv);
                representer_en_couleurs_vga (my.img_niv, my.img_coul);
            } else my.img_coul = my.img_src.clone();
        }

        if (my.need_recalc(My::R_LOUPE)) {
            // std::cout << "Calcul loupe puis affichage" << std::endl;
            my.loupe.dessiner_rect    (my.img_coul, my.img_res1);
            my.loupe.dessiner_portion (my.img_coul, my.img_res2);
            cv::imshow ("ImageSrc", my.img_res1);
            cv::imshow ("Loupe"   , my.img_res2);
        }
        my.reset_recalc();

        // Attente du prochain événement sur toutes les fenêtres, avec un
        // timeout de 15ms pour détecter les changements de flags
        int key = cv::waitKey (15);

        // Gestion des événements clavier avec une callback "maison" que l'on
        // appelle nous-même. Les Callbacks souris et slider sont directement
        // appelées par waitKey lors de l'attente.
        if (onKeyPressEvent (key, &my) < 0) break;
    }

    // Enregistrement résultat
    if (nom_out2) {
        if (! cv::imwrite (nom_out2, my.img_coul))
             std::cout << "Erreur d'enregistrement" << std::endl;
        else std::cout << "Enregistrement effectué" << std::endl;
     }
    return 0;
}

