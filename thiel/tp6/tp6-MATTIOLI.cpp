/*
    Exemples de transformations en OpenCV, avec zoom, seuil et affichage en
    couleurs. L'image de niveau est en CV_32SC1.

    g++ -Wall --std=c++14 $(pkg-config opencv --cflags)  ex01-transfos.cpp \
                          $(pkg-config opencv --libs) -o ex01-transfos
    ./ex01-transfos [-mag width height] [-thr seuil] image_in [image_out]

    CC-BY Edouard.Thiel@univ-amu.fr - 29/09/2019

                        --------------------------------

    TP5-MATTIOLI.cpp 
    Écrivez ci-dessous vos NOMS Prénoms et la date de la version :

    MATTIOLI PIERRE - version Finale TP6 15/12/2019
*/

#include <iostream>
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

// structure TP5 //

enum NumeroMasque {M_D4,M_D8,M_2_3,M_3_4,M_5_7_11,M_LAST};

struct Ponderation
{
    int y;
    int x;
    int w; //poids (a, b, ...)
    Ponderation(int y, int x, int w) {this->y=y; this->x=x; this->w=w;}
};

struct DemiMasque
{
    std::vector<Ponderation> list_pond;
    unsigned int t_pond = list_pond.size();
    NumeroMasque num_masque;
    std::string nom_masque;
    void pond_d8_poids(int a, int b)
    {
    list_pond.push_back(Ponderation(1,0,a));
    list_pond.push_back(Ponderation(1,1,b));
    list_pond.push_back(Ponderation(0,1,a));
    list_pond.push_back(Ponderation(-1,1,b));
    }
    DemiMasque(NumeroMasque num_masque)
    {
        this->num_masque = num_masque;
        list_pond.clear();
        
        switch (num_masque)
        {
        case M_D4 : 
            //std::cout << "d4" << std::endl;
            nom_masque="M_D4";
            list_pond.push_back(Ponderation(1,0,1));
            list_pond.push_back(Ponderation(0,1,1));
            break;
        case M_D8: 
            //std::cout << "d8" << std::endl;
            nom_masque="M_D8";
            pond_d8_poids(1, 1);
            break;
        case M_2_3 :
            //std::cout << "d23" << std::endl;
            nom_masque="M_2_3";
            pond_d8_poids(2, 3);
            break;
        case M_3_4 : 
            //std::cout << "d34" << std::endl;
            nom_masque="M_3_4";
            pond_d8_poids(3, 4);
            break;
        case M_5_7_11 : 
            //std::cout << "M_5_7_11" << std::endl;
            nom_masque="M_5_7_11";
            pond_d8_poids(5, 7); //poids a et b
            // poids c
            list_pond.push_back(Ponderation(2,1,11));
            list_pond.push_back(Ponderation(1,2,11));
            list_pond.push_back(Ponderation(-1,2,11));
            list_pond.push_back(Ponderation(-2,1,11));
            break;
        case M_LAST:
            //std::cout << "M_LAST" << std::endl;
            nom_masque = "M_LAST";
            break;
        default:
            break;
        }
    }
};

// Fin structure TP5 //

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
    enum Affi { A_ORIG, A_SEUIL, A_TRANS1, A_TRANS2, A_TRANS3, A_TRANS4, A_TRANS5, A_TRANS6, A_TRANS7};
    Affi affi = A_ORIG;
    
    NumeroMasque num_masque = M_D4;
    DemiMasque dm = DemiMasque(num_masque);
    int filtre = 0;
};


// *************** TP5 *************** //


void calculer_Rosenfeld_DT (cv::Mat img_niv, DemiMasque dm) //algo DT Rosenfeld
{
    //Passage Rosenfeld DT avant (avec le symétrique du demi-masque arriere)
    for (int y = 0; y < img_niv.rows; y++)
        for (int x = 0; x < img_niv.cols; x++)
        {
            if((img_niv.at<int>(y,x)!=0) && (!(x<0||y<0||y>=img_niv.rows||x>=img_niv.cols)))
            {
                for(unsigned int i=0; i < dm.list_pond.size(); i++)
                {
                    if(img_niv.at<int>(y-dm.list_pond[i].y, x-dm.list_pond[i].x) + dm.list_pond[i].w < img_niv.at<int>(y,x))
                    img_niv.at<int>(y,x) = img_niv.at<int>(y-dm.list_pond[i].y, x-dm.list_pond[i].x) + dm.list_pond[i].w;
                }
            }
        }
    //Passage Rosenfeld DT arriere (avec le demi-masque arriere)
    for (int y = img_niv.rows-1; y >= 0; y--)
        for (int x = img_niv.cols-1; x >= 0; x--)
        {
            if((img_niv.at<int>(y,x)!=0) && (!(x<0||y<0||y>=img_niv.rows||x>=img_niv.cols)))
            {
                for(unsigned int i=0; i < dm.list_pond.size(); i++)
                {
                    if(img_niv.at<int>(y+dm.list_pond[i].y, x+dm.list_pond[i].x) + dm.list_pond[i].w < img_niv.at<int>(y,x))
                    img_niv.at<int>(y,x) = img_niv.at<int>(y+dm.list_pond[i].y, x+dm.list_pond[i].x) + dm.list_pond[i].w;
                }
            }
        }
}

void calculer_Rosenfeld_RDT (cv::Mat img_niv, DemiMasque dm) //algo RDT Rosenfeld
{
    //Passage Rosenfeld DT avant (avec le symétrique du demi-masque arriere)
    for (int y = 0; y < img_niv.rows; y++)
        for (int x = 0; x < img_niv.cols; x++)
        {
            if((img_niv.at<int>(y,x)!=0) && (!(x<0||y<0||y>=img_niv.rows||x>=img_niv.cols)))
            {
                for(unsigned int i=0; i < dm.list_pond.size(); i++)
                {
                    if(img_niv.at<int>(y-dm.list_pond[i].y, x-dm.list_pond[i].x) - dm.list_pond[i].w > img_niv.at<int>(y,x))
                    img_niv.at<int>(y,x) = img_niv.at<int>(y-dm.list_pond[i].y, x-dm.list_pond[i].x) - dm.list_pond[i].w;
                }
            }
        }
    //Passage Rosenfeld DT arriere (avec le demi-masque arriere)
    for (int y = img_niv.rows-1; y >= 0; y--)
        for (int x = img_niv.cols-1; x >= 0; x--)
        {
            if((img_niv.at<int>(y,x)!=0) && (!(x<0||y<0||y>=img_niv.rows||x>=img_niv.cols)))
            {
                for(unsigned int i=0; i < dm.list_pond.size(); i++)
                {
                    if(img_niv.at<int>(y+dm.list_pond[i].y, x+dm.list_pond[i].x) - dm.list_pond[i].w > img_niv.at<int>(y,x))
                    img_niv.at<int>(y,x) = img_niv.at<int>(y+dm.list_pond[i].y, x+dm.list_pond[i].x) - dm.list_pond[i].w;
                }
            }
        }
}

void detecter_maximums_locaux (cv::Mat img_niv, DemiMasque dm)
{
    //Construction du masque complet
    DemiMasque masque_complet = DemiMasque(M_LAST);
    for(unsigned int i=0; i < dm.list_pond.size(); i++)
    {
        masque_complet.list_pond.push_back(dm.list_pond[i]); //demi-masque arriere
        masque_complet.list_pond.push_back(Ponderation(dm.list_pond[i].x*(-1), dm.list_pond[i].y*(-1), dm.list_pond[i].w*(-1))); //demi-masque avant
    }

    //Copie de img_niv dans une nouvelle image de test img_dt
    cv::Mat img_dt = cv::Mat(img_niv.rows, img_niv.cols, CV_32SC1);
    for (int y = 0; y < img_niv.rows; y++)
        for (int x = 0; x < img_niv.cols; x++)
        {
            img_dt.at<int>(y,x) = img_niv.at<int>(y,x);
        }
    
    //Calcul des maximums locaux de img_dt puis transformation sur img_niv
    for (int y = 0; y < img_dt.rows; y++)
        for (int x = 0; x < img_dt.cols; x++)
        {
            if (img_niv.at<int>(y,x)!=0)
            for(unsigned int i=0; i < masque_complet.list_pond.size(); i++)
            {
                if(img_dt.at<int>(y,x) < (img_dt.at<int>(y+masque_complet.list_pond[i].y, x+masque_complet.list_pond[i].x)) - masque_complet.list_pond[i].w)
                {
                    img_niv.at<int>(y,x) = 0;
                    break;
                }
            }
        }
}

void filtrer_formes_avec_maximums_locaux (cv::Mat img_niv, DemiMasque dm, int filtre)
{
    //copie de img_niv dans img_filtrage
    cv::Mat img_filtrage = cv::Mat(img_niv.rows, img_niv.cols, CV_32SC1);
    calculer_Rosenfeld_DT (img_niv, dm);
    detecter_maximums_locaux (img_niv, dm);
    //Couleur des points maximums locaux > filtre = cyan
    for (int y = 0; y < img_niv.rows; y++)
        for (int x = 0; x < img_niv.cols; x++)
        {
            if(img_niv.at<int>(y,x) <= filtre) 
                img_filtrage.at<int>(y,x) = 11;
        }
    calculer_Rosenfeld_RDT (img_niv, dm);
    //Couleur des points de la forme obtenue par RDT = gris
    for (int y = 0; y < img_niv.rows; y++)
        for (int x = 0; x < img_niv.cols; x++)
        {
            if(img_niv.at<int>(y,x) != 0 && img_filtrage.at<int>(y,x)!=11) 
                img_filtrage.at<int>(y,x) = 8;
        }
    //Couleur des points de la forme originale, non retrouvés par RDT = rouge
    for (int y = 0; y < img_niv.rows; y++)
        for (int x = 0; x < img_niv.cols; x++)
        {
            if(img_niv.at<int>(y,x)>0) img_filtrage.at<int>(y,x) = 12;
            else img_filtrage.at<int>(y,x) = 0;
        }
    //Copie des couleurs de img_filtrage dans l'image img_niv
    for (int y = 0; y < img_niv.rows; y++)
        for (int x = 0; x < img_niv.cols; x++)
            img_niv.at<int>(y,x) = img_filtrage.at<int>(y,x);
}

// *************** FIN TP5 *************** //

// ********** TP6 ********** //

int dE2(int ya, int xa, int yb, int xb) {return (xb-xa)*(xb-xa)+(yb-ya)*(yb-ya);}

void calculer_sedt_saito_toriwaki (cv::Mat img_niv)
{
    // *** premier passage *** //
    
    int x0,y0;
    for (int y = 0; y < img_niv.rows; y++)
    {
        for (int x = 0; x < img_niv.cols; x++) //passage gauche-droite
        {
            if ((x != img_niv.cols) && (img_niv.at<int>(y,x)==0) && (img_niv.at<int>(y,x+1)!=0))
            {
                //coord du point a 0 avant le contour
                y0=y;
                x0=x;
            }
            if (img_niv.at<int>(y,x)!=0)
            {
                img_niv.at<int>(y,x)=dE2(y0,x0,y,x);
            }
        }
        for (int x = img_niv.cols-1; x >= 0; x--) //passage droite-gauche
        {
            if ((x != 0) && (img_niv.at<int>(y,x)==0) && (img_niv.at<int>(y,x-1)!=0))
            {
                //coord du point a 0 avant le contour
                y0=y;
                x0=x;
            }
            if (img_niv.at<int>(y,x)!=0)
            {
                if ((dE2(y,x,y0,x0)) < img_niv.at<int>(y,x)) img_niv.at<int>(y,x)=dE2(y,x,y0,x0); //on prend le min entre les deux valeurs
            }
        }
    }
    
    // *** deuxieme passage *** //
        
    int val; //valeur des distances au pixel 0 + le pixel courant
    int min_col; //minimum de la colonne courante
    std::vector<int> col; //valeur des pixels de la colonne avec 0 au dessus et en dessous
    
    for (int x = 0; x < img_niv.cols; x++)
    {
        col.clear(); // R.A.Z de notre colonne
        //col.push_back(0); //on ajoute un 0 au dessus de la colonne
        for (int y = 0; y < img_niv.rows; y++)
            col.push_back(img_niv.at<int>(y,x)); //on prend les valeurs des pixels de la colonne
        //col.push_back(0); //on ajoute un 0 en dessous de la colonne
        
        for (unsigned int i = 0; i < col.size(); i++) //on ignore les 0 ajoute à la colonne
        {
            min_col=col[i]; //notre minimum est initialise a la valeur du pixel courant
            for (int yhaut = i; yhaut >= 0; yhaut--) //passage du pixel courant vers le haut de la colonne
            {
                val = col[yhaut]+dE2(i,x,yhaut,x);
                if(val < min_col) min_col = val;
                else if(dE2(i,x,yhaut,x) >= min_col) break;
            }
            for (unsigned int ybas = i; ybas <= col.size()-1; ybas++) //passage du pixel courant vers le bas de la colonne
            {
                val = col[ybas]+dE2(i,x,ybas,x);
                if(val < min_col) min_col = val;
                else if(dE2(i,x,ybas,x) >= min_col) break;
            }
            img_niv.at<int>(i,x) = min_col; //on change la couleur du pixel courant par min_col
        }
    }
}

void calculer_sedt_courbes_niveau (cv::Mat img_niv) //calcule les courbes de niveau sur l'image SEDT
{
	for (int y = 0; y < img_niv.rows; y++)
	{
		for (int x = 0; x < img_niv.cols; x++)
		{
			if (img_niv.at<int>(y, x) > 0) 
				img_niv.at<int>(y, x) = sqrt(img_niv.at<int>(y, x));
		}
	}
}

// ********** FIN TP6 ********** //

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

void transformer_bandes_horizontales (cv::Mat img_niv)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g > 0) {
            img_niv.at<int>(y,x) = y;
        }
    }
}


void transformer_bandes_verticales (cv::Mat img_niv)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g > 0) {
            img_niv.at<int>(y,x) = x;
        }
    }
}


void transformer_bandes_diagonales (cv::Mat img_niv)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g > 0) {
            img_niv.at<int>(y,x) = x+y;
        }
    }
}


// Appelez ici vos transformations selon affi
void effectuer_transformations (My::Affi affi, cv::Mat img_niv, DemiMasque dm, int filtre)
{
    switch (affi) {
        case My::A_TRANS1 :
            calculer_Rosenfeld_DT (img_niv, dm);
            break;
        case My::A_TRANS2 :
            calculer_Rosenfeld_DT (img_niv, dm);
            detecter_maximums_locaux (img_niv, dm);
            break;
        case My::A_TRANS3 :
            calculer_Rosenfeld_DT (img_niv, dm);
            detecter_maximums_locaux (img_niv, dm);
            calculer_Rosenfeld_RDT (img_niv, dm);
            break;
        case My::A_TRANS4 :
            filtrer_formes_avec_maximums_locaux (img_niv, dm, filtre);
            break;
        case My::A_TRANS5 :
            calculer_sedt_saito_toriwaki (img_niv);
            break;
        case My::A_TRANS6 :
            calculer_sedt_saito_toriwaki(img_niv);
            calculer_sedt_courbes_niveau(img_niv);
            break;
        case My::A_TRANS7 :
            calculer_sedt_saito_toriwaki(img_niv);
            calculer_sedt_courbes_niveau(img_niv);
            detecter_maximums_locaux (img_niv, dm);
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

void onFiltreSlide (int pos, void *data)
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
        "   1    transformation DT Rosenfeld\n"
        "   2    transformation Maximums locaux\n"
        "   3    transformation RDT Rosenfeld\n"
        "   4    transformation Filtrage des maximum locaux\n"
        "   5    transformation SEDT_saito_toriwaki\n"
        "   6    transformation courbes de niveau sur l'image SEDT\n"
        "   7    transformation Maximums locaux SEDT\n"
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

        // Rajoutez ici des touches pour les transformations.
        // Dans my->set_recalc, passez :
        //   My::R_SEUIL pour faire le calcul à partir de l'image originale seuillée
        //   My::R_TRANSFOS pour faire le calcul à partir de l'image actuelle
        case '1' :
            std::cout << "Rosenfeld DT" << std::endl;
            my->affi = My::A_TRANS1;
            my->set_recalc(My::R_SEUIL);
            break;
        case '2' :
            std::cout << "Maximums locaux" << std::endl;
            my->affi = My::A_TRANS2;
            my->set_recalc(My::R_SEUIL);
            break;
        case '3' :
            std::cout << "Rosenfeld RDT" << std::endl;
            my->affi = My::A_TRANS3;
            my->set_recalc(My::R_SEUIL);
            break;
        case '4' :
            std::cout << "Filtrage des maximum locaux" << std::endl;
            my->affi = My::A_TRANS4;
            my->set_recalc(My::R_SEUIL);
            break;
        case '5' :
            std::cout << "SEDT Saito Toriwaki" << std::endl;
            my->affi = My::A_TRANS5;
            my->set_recalc(My::R_SEUIL);
            break;
        case '6' :
            std::cout << "courbes de niveau sur l'image SEDT" << std::endl;
            my->affi = My::A_TRANS6;
            my->set_recalc(My::R_SEUIL);
            break;
        case '7' :
            std::cout << "Maximums locaux SEDT" << std::endl;
            my->affi = My::A_TRANS6;
            my->set_recalc(My::R_SEUIL);
            break;
        case 'd' :
            my->num_masque = NumeroMasque(my->num_masque+1);
            if(my->num_masque == M_LAST) my->num_masque = M_D4;
            my->dm = DemiMasque(my->num_masque);
            std::cout << "basculement vers le masque " << my->dm.nom_masque << std::endl;
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
    cv::createTrackbar ("Filtre", "ImageSrc", &my.filtre, 300,
        onFiltreSlide, &my);
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
                effectuer_transformations (my.affi, my.img_niv, my.dm, my.filtre);
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

