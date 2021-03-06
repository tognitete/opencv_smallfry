#ifndef __lbptop_onboard__

#include <opencv2/opencv.hpp>
#include <deque>


typedef std::deque< cv::Mat_<uchar> > Sequence;

void lbp_xy(const cv::Mat_<uchar> &img, cv::Mat &hist, const cv::Rect &rec);

void lbp_xy(const Sequence &seq, cv::Mat &hist, const cv::Rect &rec);
void lbp_xz(const Sequence &seq, cv::Mat &hist, const cv::Rect &rec);
void lbp_yz(const Sequence &seq, cv::Mat &hist, const cv::Rect &rec);

cv::Mat lbptop(const Sequence &seq);

cv::Mat img_yz(const Sequence &seq, const cv::Rect &r);
cv::Mat img_xz(const Sequence &seq, const cv::Rect &r);

int lbpFlow(const cv::String &filename, cv::Mat &desc, int frameFrom=0, int frameTo=0);


#endif // __lbptop_onboard__
