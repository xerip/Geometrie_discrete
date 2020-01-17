/*
    L'image de niveau est en CV_32SC1.

    g++ -Wall --std=c++14 $(pkg-config opencv --cflags)  tp7.cpp \
                          $(pkg-config opencv --libs) -o tp7
    ./tp7 [-mag width height] [-thr seuil] image_in eltStruct [image_out]

    CC-BY Edouard.Thiel@univ-amu.fr - 29/09/2019

                        --------------------------------

    MABILY Johan & MATTIOLI Pierre - version du 04/12/2019
*/

#include <iostream>
#include <cstring>
#include <opencv2/opencv.hpp>

#include "Contour.h"

#include <vector>
using namespace std;

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

//------------------------ M E S    D O N N E E S -----------------------------

const int g_nx8[] = {1, 1, 0, -1, -1, -1, 0, 1};
const int g_ny8[] = {0, 1, 1, 1, 0, -1, -1, -1};
enum enum_type_forme { FORME_0, FORME_255};
enum enum_algo_type { ITERATIF, RECURSIF};
int g_typeAlgo = ITERATIF;
bool g_chargement = false;
const int LIMITE_ELT_STRUCT = 50;
void rotate_eltStruct(cv::Mat img);
void adapt_eltStruct(cv::Mat img_eltStruct, cv::Point &centre);

//----------------------------------- M Y -------------------------------------

class My {
  public:
    cv::Mat img_src, img_res1, img_res2, img_niv, img_coul;
    cv::Mat img_srcES, img_res1ES, img_res2ES, img_eltStruct, img_coulES;
    Loupe loupe;
    int seuil = 127;
    int clic_x = 0;
    int clic_y = 0;
    int clic_n = 0;
    int filtre = 0;

    enum Recalc { R_RIEN, R_LOUPE, R_TRANSFOS, R_SEUIL };
    Recalc recalc = R_SEUIL;

    void reset_recalc ()             { recalc = R_RIEN; }
    void set_recalc   (Recalc level) { if (level > recalc) recalc = level; }
    int  need_recalc  (Recalc level) { return level <= recalc; }

    // Rajoutez ici des codes A_TRANSx pour le calcul et l'affichage
    enum Affi { A_ORIG, A_SEUIL, A_TRANS1, A_TRANS2, A_TRANS3, A_TRANS4,
		 A_TRANS5, A_TRANS6, A_TRANS7, A_TRANS8 };
    Affi affi = A_ORIG;

};


//----------------------------- U T I L I T A I R E S --------------------------

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

void inverser_code_couleur(cv::Mat img)
{
    for (int y = 0; y < img.rows; y++)
    for (int x = 0; x < img.cols; x++)
    {
        if (img.at<int>(y, x) == 255) {
            img.at<int>(y, x) = 0;
        }
        else {
            img.at<int>(y, x) = 255;
        }
    }
}

/*--------------------------------------------------------------
* Vrai si la coord (@x, @y) se trouve dans le plan de l'image
* ------------------------------------------------------------*/
bool est_dans_image(cv::Mat img_niv, int x, int y)
{
	if ( x<0 || x>= img_niv.cols
		|| y<0 || y>= img_niv.rows)
	{
		return false;
	}
	return true;
}

void recolor(cv::Mat img_niv, int color)
{
	for (int y = 0; y < img_niv.rows; y++)
	{
		for (int x = 0; x < img_niv.cols; x++)
		{
			if (img_niv.at<int>(y, x) != 255) {
				img_niv.at<int>(y, x) = color;
			}
		}
	}
}

bool est_dans_forme(cv::Mat img, cv::Point P, int typeForme)
{
    int val;
    if (typeForme == FORME_0) {
        val = 0;
    }
    else {
        val = 255;
    }
    if (img.at<int>(P.y, P.x) == val)
        return true;
    return false;
}

/*--------------------------------------------------------------
 * Vrai si @P2 est dans l'élément structurant calqué sur @P1.
 * (dont le centre @centreES est calqué sur @P1)
* ------------------------------------------------------------*/
bool est_dans_eltStruct(cv::Mat eltStruct, cv::Point centreES,
        cv::Point P1, cv::Point P2)
{
    if (P1 == P2)   return true;
    cv::Point chemin = P1 - P2;
    cv::Point test = centreES - chemin;
    if (!est_dans_image(eltStruct, test.x, test.y)) return false;
    // cout << __FUNCTION__ << ": eltStruct.at<int>(" << test.y << ", " << test.x << ") = " << eltStruct.at<int>(test.y, test.x) << endl;
    if (eltStruct.at<int>(test.y, test.x) == 0)   return true;
    return false;
}

bool point_est_dans_vector(vector<cv::Point> v, cv::Point P)
{
    for (unsigned i=0; i<v.size(); i++)
    {
        if (v[i] == P)  return true;
    }
    return false;
}


//----------------- D I L A T A T I O N  /  E R O S I O N ----------------------

/*--------------------------------------------------------------
 * Vrai si l'élément structurant @eltStruct calqué sur
 * @P1, un point de @img_niv, a au moins un pixel @P2 qui se
 * calque sur la forme dans @img_niv.
 * (Fonction récursive)
* ------------------------------------------------------------*/
bool intersection_test_rec(cv::Mat img_niv, cv::Mat eltStruct,
        cv::Point centreES,
        cv::Point P1, cv::Point P2, vector<cv::Point> &pointsVisites, int prof)
{
    // cout << "prof = " << prof << endl;
    pointsVisites.push_back(P2);
    if ( !est_dans_image(img_niv, P2.x, P2.y)
        ||  !est_dans_eltStruct(eltStruct, centreES, P1, P2)) {
        return false;
    }
    if ( est_dans_image(img_niv, P2.x, P2.y)
        && est_dans_eltStruct(eltStruct, centreES, P1, P2)
        &&  est_dans_forme(img_niv, P2, FORME_0))
    {
        return true;
    }
    else
    {
        for (int dir = 0; dir < 8; dir++)
        {
            int xn = P2.x + g_nx8[dir];
            int yn = P2.y + g_ny8[dir];
            cv::Point PN(xn, yn);
            if (!point_est_dans_vector(pointsVisites, PN))
            {
                if (intersection_test_rec(img_niv, eltStruct,
                    centreES, P1, PN,
                    pointsVisites, prof+1)) {
                    return true;
                }
            }
        }
        return false;
    }
}

/*--------------------------------------------------------------
 * Vrai si l'élément structurant @eltStruct calqué sur
 * @P1, un point de @img_niv, a au moins un pixel @P2 qui se
 * calque sur la forme dans @img_niv.
 * (Fonction itérative)
* ------------------------------------------------------------*/
bool intersection_test_iteratif(cv::Mat img_niv, cv::Mat eltStruct,
        cv::Point centreES, cv::Point P1)
{
    cv::Point P2;
    for (int y = 0; y < img_niv.rows; y++)
	{
		for (int x = 0; x < img_niv.cols; x++)
		{
            P2 = cv::Point(x, y);
            if (est_dans_forme(img_niv, P2, FORME_0)
                && est_dans_eltStruct(eltStruct, centreES, P1, P2))
            {
                return true;
            }
        }
    }
    return false;
}

void dilatation(cv::Mat img_niv, cv::Mat eltStruct, cv::Point centreES)
{
    if (g_chargement) cout << "\t\t<" << __FUNCTION__ << ">" << endl;
	float xn, yn;
	cv::Mat tmp;
	img_niv.copyTo(tmp);
	img_niv.setTo(255);
    bool intersect = false;
    vector<cv::Point> pointsVisites;

	for (int y = 1; y < tmp.rows-1; y++)
	{
		if (g_chargement) cout << "\t\t\t" << y << "/" << tmp.rows-1 << endl;

		for (int x = 1; x < tmp.cols-1; x++)
		{
            cv::Point P1(x, y);
            if (g_typeAlgo == ITERATIF) {
                intersect = intersection_test_iteratif(tmp, eltStruct, centreES, P1);
            }
            else if (g_typeAlgo >= RECURSIF)
            {
                pointsVisites.clear();
                intersect = intersection_test_rec(tmp, eltStruct,
                    centreES, P1, P1,
                    pointsVisites, 0);
            }
            if (intersect)  img_niv.at<int>(y, x) = 0;
		}
	}
    if (g_chargement) cout << "\t\t</" << __FUNCTION__ << ">" << endl;
}

bool inclusion_test_iteratif(cv::Mat img_niv, cv::Mat eltStruct,
        cv::Point centreES, cv::Point P1)
{
    cv::Point P2;
    for (int y = 0; y < img_niv.rows; y++)
	{
		for (int x = 0; x < img_niv.cols; x++)
		{
            P2 = cv::Point(x, y);
            if (!est_dans_forme(img_niv, P2, FORME_0)
                && est_dans_eltStruct(eltStruct, centreES, P1, P2))
            {
                return false;
            }
        }
    }
    return true;
}

bool inclusion_test_rec(cv::Mat img_niv, cv::Mat eltStruct,
        cv::Point centreES,
        cv::Point P1, cv::Point P2, vector<cv::Point> &pointsVisites)
{
    pointsVisites.push_back(P2);
    if ( est_dans_image(img_niv, P2.x, P2.y)
        &&  est_dans_eltStruct(eltStruct, centreES, P1, P2))
    {
        if (!est_dans_forme(img_niv, P2, FORME_0)) {
            return false;
        }
        else
        {
            for (int dir = 0; dir < 8; dir++)
            {
                int xn = P2.x + g_nx8[dir];
                int yn = P2.y + g_ny8[dir];
                cv::Point PN(xn, yn);
                if (!point_est_dans_vector(pointsVisites, PN))
                {
                    if (!inclusion_test_rec(img_niv, eltStruct,
                        centreES, P1, PN,
                        pointsVisites))
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    }
    return true;
}

void erosion(cv::Mat img_niv, cv::Mat eltStruct, cv::Point centreES)
{
	if (g_chargement) cout << "\t\t<" << __FUNCTION__ << ">" << endl;
    cv::Mat tmp;
    img_niv.copyTo(tmp);
    img_niv.setTo(255);
    vector<cv::Point> pointsVisites;
    bool inclus;
    for (int y = 1; y < tmp.rows-1; y++)
	{
		if (g_chargement) cout << "\t\t\t" << y << "/" << tmp.rows-1 << endl;
		for (int x = 1; x < tmp.cols-1; x++)
		{
            cv::Point P1(x, y);
            if (g_typeAlgo == ITERATIF) {
                inclus = inclusion_test_iteratif(tmp, eltStruct, centreES, P1);
            }
            else if (g_typeAlgo >= RECURSIF) {
                pointsVisites.clear();
                inclus = inclusion_test_rec(tmp, eltStruct,
                    centreES, P1, P1,
                    pointsVisites);
            }
            if (inclus) {
                img_niv.at<int>(y, x) = 0;
            }
        }
    }
    if (g_chargement) cout << "\t\t<" << __FUNCTION__ << ">" << endl;
}


//------------------------- O P E R A T I O N S --------------------------------

void ouverture(cv::Mat img_niv, cv::Mat eltStruct, cv::Point centreES)
{
	erosion(img_niv, eltStruct, centreES);
    rotate_eltStruct(eltStruct);
    adapt_eltStruct(eltStruct, centreES);
	dilatation(img_niv, eltStruct, centreES);
}

void fermeture (cv::Mat img_niv, cv::Mat eltStruct, cv::Point centreES)
{
	dilatation(img_niv, eltStruct, centreES);
    rotate_eltStruct(eltStruct);
    adapt_eltStruct(eltStruct, centreES);
	erosion(img_niv, eltStruct, centreES);
}

void gradient_inf(cv::Mat img_niv, cv::Mat eltStruct, cv::Point centreES)
{
	cv::Mat img_eros;
	img_niv.copyTo(img_eros);
	erosion(img_eros, eltStruct, centreES);

	for (int y = 0; y < img_eros.rows; y++)
	{
		for (int x = 0; x < img_eros.cols; x++)
		{
			if (img_eros.at<int>(y, x) != 255) {
				img_niv.at<int>(y, x) = 255;
			}
		}
	}
}

void gradient_externe(cv::Mat img_niv, cv::Mat eltStruct, cv::Point centreES)
{
	cv::Mat img_originale;
	img_niv.copyTo(img_originale);
	dilatation(img_niv, eltStruct, centreES);

	for (int y = 0; y < img_niv.rows; y++)
	{
		for (int x = 0; x < img_niv.cols; x++)
		{
			if (img_niv.at<int>(y, x) == 0
				&& img_originale.at<int>(y, x) == 0)
			{
				img_niv.at<int>(y, x) = 255;
			}
		}
	}
}

void gradient_morphologique(cv::Mat img_niv, cv::Mat eltStruct, cv::Point centreES)
{
	cv::Mat img_erosion;
	img_niv.copyTo(img_erosion);
	erosion(img_erosion, eltStruct, centreES);

	dilatation(img_niv, eltStruct, centreES);

	for (int y = 0; y < img_niv.rows; y++)
	{
		for (int x = 0; x < img_niv.cols; x++)
		{
			if (img_niv.at<int>(y, x) == 0
				&& img_erosion.at<int>(y, x) == 0)
			{
				img_niv.at<int>(y, x) = 255;
			}
		}
	}
}

void laplacien_morphologique(cv::Mat img_niv, cv::Mat eltStruct, cv::Point centreES)
{
	cv::Mat img_gInf;
	img_niv.copyTo(img_gInf);
	gradient_inf(img_gInf, eltStruct, centreES);

	gradient_externe(img_niv, eltStruct, centreES);

	for (int y = 0; y < img_niv.rows; y++)
	{
		for (int x = 0; x < img_niv.cols; x++)
		{
			if (img_niv.at<int>(y, x) == 0
				&& img_gInf.at<int>(y, x) == 0)
			{
				img_niv.at<int>(y, x) = 255;
			}
		}
	}
}


//-------------------- T R A N S F O R M A T I O N S ---------------------------

void rotate_eltStruct(cv::Mat img)
{
    cv::Mat tmp;
	img.copyTo(tmp);
    img.setTo(255);
    for (int y = 0; y < tmp.rows; y++)
	{
		for (int x = 0; x < tmp.cols; x++)
		{
            if (tmp.at<int>(y, x) != 0) continue;
            int newX =  tmp.cols - x;
            int newY = tmp.rows - y;
            img.at<int>(newY, newX) = 0;
        }
    }
}

/*-------------------------------------------------------------------------
 * ATTENTION -> A n'utiliser que sur des images
 * dont le bord ne contient pas la forme.
 * =============================================
 * Adapte l'image choisie comme élément structurant,
 * aux règles de ce programme,
 * en recoloriant la forme et le fond si besoin.
 * Renvoi également le centre de l'élément structurant.
 *-------------------------------------------------------------------------*/
void adapt_eltStruct(cv::Mat img_eltStruct, cv::Point &centre)
{
    // Si fond à 0 on inverse les couleurs
    if (img_eltStruct.at<int>(0, 0) == 0) {
        inverser_code_couleur(img_eltStruct);
    }

    cv::Mat tmp;
    img_eltStruct.copyTo(tmp);
    // La classe Contour considère la forme pour les pixels de valeur = 255
    int typeForme = FORME_255;
    inverser_code_couleur(tmp);
	Contour myC(tmp);
    myC.calc_contours();
    centre = myC.get_centre();
}

// Appelez ici vos transformations selon affi
void effectuer_transformations (My::Affi affi, cv::Mat img_niv, cv::Mat img_eltStruct, int filtre)
{
    cout << "\t<" << __FUNCTION__ << ">" << endl;
    cv::Point centreES;
    adapt_eltStruct(img_eltStruct, centreES);

    switch (affi) {
        case My::A_TRANS1 :
            //transformer_bandes_horizontales (img_niv);
            dilatation(img_niv, img_eltStruct, centreES);
            //recolor(img_niv, 0);
            break;
        case My::A_TRANS2 :
            //~ transformer_bandes_verticales (img_niv);
            erosion(img_niv, img_eltStruct, centreES);
            break;
        case My::A_TRANS3 :
			ouverture(img_niv, img_eltStruct, centreES);
            //~ transformer_bandes_diagonales (img_niv);
            break;
		case My::A_TRANS4 :
			fermeture(img_niv, img_eltStruct, centreES);
			break;
		case My::A_TRANS5 :
			gradient_inf(img_niv, img_eltStruct, centreES);
			break;
		case My::A_TRANS6 :
			gradient_externe(img_niv, img_eltStruct, centreES);
			break;
		case My::A_TRANS7 :
			gradient_morphologique(img_niv, img_eltStruct, centreES);
			break;
		case My::A_TRANS8 :
			laplacien_morphologique(img_niv, img_eltStruct, centreES);
			break;
        default : break;
    }
    cout << "\t</" << __FUNCTION__ << ">" << endl;
    cout << endl;
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
    //~ if (my->filtre == 0) my->filtre = 1;
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

        "\n"

        "   r    change le type d'algo : itératif ou récursif\n"
        "   c    affiche le chargement des transformations\n"

        "\n"

        "   1    affiche la dilatation\n"
        "   2    affiche l'érosion\n"
        "   3    affiche l'ouverture\n"
        "   4    affiche la fermeture\n"
        "   5    affiche le gradient inférieur\n"
        "   6    affiche le gradient externe\n"
        "   7    affiche le laplacien\n"
        "   8    affiche le laplacien morphologique\n"
        "  esc   quitte\n"
    << std::endl;
}

// Callback "maison" pour le clavier
int onKeyPressEvent (int key, void *data)
{
    My *my = (My*) data;
    //int num;

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
        case 'r' :
            std::cout << "type algo : ";
            if (g_typeAlgo == ITERATIF) {
                g_typeAlgo = RECURSIF;
                std::cout << "récursif" << endl;
            }
            else {
                g_typeAlgo = ITERATIF;
                std::cout << "itératif" << endl;
            }
            //~ my->affi = My::A_SEUIL;
            //~ my->set_recalc(My::R_SEUIL);
            break;
		case 'c' :
			if (g_chargement) {
				g_chargement = false;
				std::cout << "chargement désactivé" << endl;
			}
			else {
				g_chargement = true;
				std::cout << "chargement activé" << endl;
			}
            break;

        // Rajoutez ici des touches pour les transformations.
        // Dans my->set_recalc, passez :
        //   My::R_SEUIL pour faire le calcul à partir de l'image originale seuillée
        //   My::R_TRANSFOS pour faire le calcul à partir de l'image actuelle
        case '1' :
            std::cout << "Dilatation" << std::endl;
            my->affi = My::A_TRANS1;
            my->set_recalc(My::R_SEUIL);
            break;
        case '2' :
            std::cout << "Erosion" << std::endl;
            my->affi = My::A_TRANS2;
            my->set_recalc(My::R_SEUIL);
            break;
        case '3' :
            std::cout << "Ouverture" << std::endl;
            my->affi = My::A_TRANS3;
            my->set_recalc(My::R_SEUIL);
            break;
        case '4' :
            std::cout << "Fermeture" << std::endl;
            my->affi = My::A_TRANS4;
            my->set_recalc(My::R_SEUIL);
            break;
        case '5' :
            std::cout << "Gradient Inf" << std::endl;
            my->affi = My::A_TRANS5;
            my->set_recalc(My::R_SEUIL);
            break;
        case '6' :
            std::cout << "Gradient Externe" << std::endl;
            my->affi = My::A_TRANS6;
            my->set_recalc(My::R_SEUIL);
            break;
        case '7' :
            std::cout << "Gradient Morphologique" << std::endl;
            my->affi = My::A_TRANS7;
            my->set_recalc(My::R_SEUIL);
            break;
        case '8' :
            std::cout << "Laplacien Morphologique" << std::endl;
            my->affi = My::A_TRANS8;
            my->set_recalc(My::R_SEUIL);
            break;

        default :
            //std::cout << "Touche '" << char(key) << "'" << std::endl;
            break;
    }
    return 1;
}

void choose_type_algo(cv::Mat img_niv, cv::Mat eltStruct)
{
	int imgTaille = img_niv.rows + img_niv.cols;
	int eSTaille = eltStruct.rows + eltStruct.cols;
	imgTaille/= 10;

	cout << "type algo : ";
	if (eSTaille <= LIMITE_ELT_STRUCT
		|| eSTaille < imgTaille) {
		g_typeAlgo = RECURSIF;
		std::cout << "récursif" << endl;
	}
	else {
		g_typeAlgo = ITERATIF;
		std::cout << "itératif" << endl;
	}
}

//---------------------------------- M A I N ----------------------------------

void afficher_usage (char *nom_prog) {
    std::cout << "Usage: " << nom_prog
              << "[-mag width height] [-thr seuil] in1 eltStruct [out2] [typeAlgo: -i | -r]"
              << std::endl;
}

int main (int argc, char**argv)
{
    My my;
    //~ cout << "my.currDemiMask->nom = " << my.currDemiMask->nom << endl;
    char *nom_in1, *nom_out2, *nom_prog = argv[0];
    char *nom_eltStruct;
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
    if (argc-1 < 1 or argc-1 > 4) { afficher_usage(nom_prog); return 1; }
    nom_in1  = argv[1];
    nom_eltStruct = argv[2];
    if (argv[3]!=nullptr)
    {
        if (strcmp(argv[3], "-i") == 0) {
            g_typeAlgo = ITERATIF;
        }
        else if (strcmp(argv[3], "-r") == 0) {
            g_typeAlgo = RECURSIF;
        }
        else {
            nom_out2 = (argc-1 == 3) ? argv[3] : NULL;
        }
    }

    // Lecture image
    my.img_src = cv::imread (nom_in1, cv::IMREAD_COLOR);  // produit du 8UC3
    if (my.img_src.empty()) {
        std::cout << "Erreur de lecture" << std::endl;
        return 1;
    }
    my.img_srcES = cv::imread (nom_eltStruct, cv::IMREAD_COLOR);  // produit du 8UC3
    if (my.img_srcES.empty()) {
        std::cout << "Erreur de lecture" << std::endl;
        return 1;
    }

    // Création résultats
    my.img_res1 = cv::Mat(my.img_src.rows, my.img_src.cols, CV_8UC3);
    my.img_res2 = cv::Mat(zoom_h, zoom_w, CV_8UC3);
    my.img_niv  = cv::Mat(my.img_src.rows, my.img_src.cols, CV_32SC1);
    my.img_coul = cv::Mat(my.img_src.rows, my.img_src.cols, CV_8UC3);

    my.img_res1ES = cv::Mat(my.img_srcES.rows, my.img_srcES.cols, CV_8UC3);
    // my.img_res2ES = cv::Mat(zoom_h, zoom_w, CV_8UC3);
    my.img_eltStruct  = cv::Mat(my.img_srcES.rows, my.img_srcES.cols, CV_32SC1);
    // my.img_coulES = cv::Mat(my.img_srcES.rows, my.img_srcES.cols, CV_8UC3);

    my.loupe.reborner(my.img_res1, my.img_res2);

    // Création fenêtre
    cv::namedWindow ("ImageSrc", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar ("Zoom", "ImageSrc", &my.loupe.zoom, my.loupe.zoom_max,
        onZoomSlide, &my);
    cv::createTrackbar ("Seuil", "ImageSrc", &my.seuil, 255,
        onSeuilSlide, &my);
	cv::createTrackbar ("Filtre", "ImageSrc", &my.filtre, 500,
        onFiltreSlide, &my);
    cv::setMouseCallback ("ImageSrc", onMouseEvent, &my);

    cv::namedWindow ("Loupe", cv::WINDOW_AUTOSIZE);
    afficher_aide();

	choose_type_algo(my.img_niv, my.img_eltStruct);


    // Boucle d'événements
    for (;;) {

        if (my.need_recalc(My::R_SEUIL))
        {
            // std::cout << "Calcul seuil" << std::endl;
            cv::Mat img_gry;
            cv::cvtColor (my.img_src, img_gry, cv::COLOR_BGR2GRAY);
            cv::threshold (img_gry, img_gry, my.seuil, 255, cv::THRESH_BINARY);
            img_gry.convertTo (my.img_niv, CV_32SC1,1., 0.);

            cv::Mat img_gryES;
            cv::cvtColor (my.img_srcES, img_gryES, cv::COLOR_BGR2GRAY);
            cv::threshold (img_gryES, img_gryES, my.seuil, 255, cv::THRESH_BINARY);
            img_gryES.convertTo (my.img_eltStruct, CV_32SC1,1., 0.);
        }

        if (my.need_recalc(My::R_TRANSFOS))
        {
            // std::cout << "Calcul transfos" << std::endl;
            if (my.affi != My::A_ORIG) {
                effectuer_transformations (my.affi, my.img_niv, my.img_eltStruct, my.filtre);
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
