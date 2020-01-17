#ifndef CONTOUR_H
#define CONTOUR_H

#include <iostream>
#include <cstring>
#include <opencv2/opencv.hpp>

#include <vector>
using namespace std;

class Contour
{
public:

	Contour (cv::Mat &img);
	~Contour();

	cv::Mat _img_niv();

	void diplay_matrice_img();
	void calc_contours();
	cv::Point get_centre();

private:

	cv::Mat &img_niv;

	bool est_dans_image(cv::Mat img_niv, int x, int y);
	int square_distance_euclid(cv::Point A, cv::Point B);

	bool get_boundaries (cv::Mat img_niv, cv::Point P, cv::Point &B1, cv::Point &B2);
	void sedt_balayageX();
	int search_min_dist_square_of_col(cv::Mat myCol, int x, int y);
	void sedt_balayageY();
	void calculer_sedt_saito_toriwaki ();
	void calculer_sedt_courbes_niveau ();
};

#endif // CONTOUR_H
