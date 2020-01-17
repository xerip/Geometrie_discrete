#include "Contour.h"

//------------------------ M E S    D O N N E E S -----------------------------

enum Side {FRONT, BACK};

//------------------------ C L A S S E     C O N T O U R -----------------------

Contour::Contour (cv::Mat &img) :
	img_niv(img)
{
	;
}

Contour::~Contour()
{
	;
}

cv::Mat Contour::_img_niv() {	return img_niv;	}

void Contour::diplay_matrice_img()
{
	for (int y = 0; y < img_niv.rows; y++)
	{
		for (int x = 0; x < img_niv.cols; x++)
		{
			cout << "Mat(" << x << ", " << y << ") = " << img_niv.at<int>(y, x) << endl;
		}
	}
}

void Contour::calc_contours()
{
	calculer_sedt_saito_toriwaki();
	calculer_sedt_courbes_niveau();
}

/*-------------------------------------------------------------------------
 * ATTENTION -> La méthode ne devrait être appelé qu'après avoir
 * fait un appel à @calc_contours()
 * ===================================
 * Renvoi le premier pixel trouvé avec le label le plus élevé.
 *-------------------------------------------------------------------------*/
cv::Point Contour::get_centre()
{
	cv::Point centre;
    int coulMax = -1;
    for (int y = 0; y < img_niv.rows; y++)
	{
		for (int x = 0; x < img_niv.cols; x++)
		{
			if (img_niv.at<int>(y, x) > 0)
            {
                if(coulMax < img_niv.at<int>(y, x)) {
                    coulMax = img_niv.at<int>(y, x);
                    centre = cv::Point(x, y);
                }
            }
		}
	}
    return centre;
}

/*--------------------------------------------------------------
 * Vrai si la coord (@x, @y) se trouve dans le plan de l'image
 * ------------------------------------------------------------*/
bool Contour::est_dans_image(cv::Mat img_niv, int x, int y)
{
	if ( x<0 || x>= img_niv.cols
		|| y<0 || y>= img_niv.rows)
	{
		return false;
	}
	return true;
}

int Contour::square_distance_euclid(cv::Point A, cv::Point B)
{
	return ( (B.x - A.x)*(B.x - A.x)) + ((B.y - A.y)*(B.y - A.y) );
}

/*--------------------------------------------------------------
 * Renvoie les 2 points @B1 et @B2 qui bordent une forme
 * sur la ligne du point @P (s'il appartient à une forme
 * @inForm)
 * -------------------------------------------------------------*/
bool Contour::get_boundaries (cv::Mat img_niv, cv::Point P, cv::Point &B1, cv::Point &B2)
{
	if (img_niv.at<int>(P.y, P.x) > 0)
	{
		B1 = cv::Point(P);
		while (B1.x>= 0 && img_niv.at<int>(B1.y, B1.x) > 0)
		{
			// Même si on sort de l'image ça ne devrait pas poser de bleme avec le calcul
			B1.x = B1.x-1;
		}

		B2 = cv::Point(P);
		while (B2.x < img_niv.cols && img_niv.at<int>(B2.y, B2.x) > 0)
		{
			B2.x = B2.x+1;
		}

		return true;
	}
	return false;
}

/*----------------------------------------------------------------
 * Pour chaque ligne, marque chaque pixel de la ligne à sa
 * distance euclidienne au carré, au 0 le plus proche sur la ligne
 * ---------------------------------------------------------------*/
void Contour::sedt_balayageX()
{
	bool inForm = false;
	cv::Point P1, P2;

	for (int y = 0; y < img_niv.rows; y++)
	{
		for (int x = 0; x < img_niv.cols; x++)
		{
			cv::Point P(x, y);
			if (!inForm && img_niv.at<int>(y, x) > 0) {
				if (!get_boundaries(img_niv, P, P1, P2)) {
					cerr << "mauvaise utilisation de " << __FUNCTION__ << ", return false" << endl;
					exit (1);
				}
				inForm = true;
			}
			else if (inForm && img_niv.at<int>(y, x) <= 0) {
				inForm = false;
			}
			if (inForm)
			{
				int distE1 = square_distance_euclid(P, P1);
				int distE2 = square_distance_euclid(P, P2);
				img_niv.at<int>(y, x) = min(distE1, distE2);
			}
		}
		inForm = false;
	}
}

/*----------------------------------------------------------------
 * Cherche le min de la valeur du pixel
 * à la position @y de la colonne @myCol + sa distance euclidienne
 * au carré du 0 le plus proche sur la colonne.
 * ---------------------------------------------------------------*/
int Contour::search_min_dist_square_of_col(cv::Mat myCol, int x, int y)
{
	int squareDist = 0;
	int min = myCol.at<int>(y, x);
	cv::Point P(x, y);

	// Explore en haut
	for (int i=y-1; i>=-1; i--)
	{
		cv::Point P1(x, i);
		squareDist = square_distance_euclid(P, P1);
		if (squareDist >= min) break;
		int valPix;
		if (i < 0)	valPix = 0;
		else 		valPix = myCol.at<int>(i, x);
		int res = squareDist + valPix;
		if (min >= res)	min = res;

	}
	// Explore en bas
	for (int i=y+1; i<myCol.rows+1; i++)
	{
		cv::Point P1(x, i);
		squareDist = square_distance_euclid(P, P1);
		if (squareDist >= min) break;
		int valPix;
		if (i >= myCol.rows)	valPix = 0;
		else 					valPix = myCol.at<int>(i, x);
		int res = squareDist + valPix;
		if (min >= res)	min = res;
	}

	return min;
}

/*----------------------------------------------------------------
 * Algorithme de valeur minimale avec distance euclidienne
 * appliqué à chaque colonne
 * ---------------------------------------------------------------*/
void Contour::sedt_balayageY()
{
	// cout << "\t\t<" << __FUNCTION__ << ">" << endl;
	cv::Mat myCol;

	for (int x = 0; x < img_niv.cols; x++)
	{
		// cout << "\t\t\t" << img_niv.at<int>(0, 0) << endl;
		// cout << "\t\t\tTEST A" << endl;
		img_niv.copyTo(myCol);
		// cout << "\t\t\tTEST B" << endl;
		for (int y = 0; y < img_niv.rows; y++)
		{
			int min = search_min_dist_square_of_col(myCol, x, y);
			img_niv.at<int>(y, x) = min;
		}
	}
	// cout << "\t\t<" << __FUNCTION__ << ">" << endl;
}

void Contour::calculer_sedt_saito_toriwaki ()
{
	//cout << "img_niv.cols = " << img_niv.cols << endl;
	sedt_balayageX();
	sedt_balayageY();
}

void Contour::calculer_sedt_courbes_niveau ()
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
