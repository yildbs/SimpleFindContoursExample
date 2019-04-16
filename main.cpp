#include <vector>
#include <deque>
#include <memory>

#include <cv.h>
#include <opencv2/highgui/highgui.hpp>

template<typename T>
class YFrame{
public:
	YFrame(int width, int height)
		: width(width)
		, height(height)
	{
		this->data = std::shared_ptr<T>(new T[width*height]);
		memset((void*)this->data.get(), 0, width*height*sizeof(T));
	}
	inline T& Get(int x, int y){
		return this->data.get()[y*this->width + x];
	}

public:
	int width;
	int height;
	std::shared_ptr<T> data;
};

class YValidatedPoint{
public:
	YValidatedPoint()
		: x(0)
		, y(0)
		, valid(true)
	{ }
	YValidatedPoint(int x, int y)
		: x(x)
		, y(y)
		, valid(true)
	{ }
	YValidatedPoint(int x, int y, int xmin, int xmax, int ymin, int ymax) //validate
		: x(x)
		, y(y)
		, valid(true)
	{ 
		if (x < xmin){
			valid = false;
		}
		else if (xmax <= x){
			valid = false;
		}
		else if (y < ymin){
			valid = false;
		}
		else if (ymax <= y){
			valid = false;
		}
	}
	int X(){
		return this->x;
	}
	int Y(){
		return this->y;
	}
	bool IsValid(){
		return this->valid;
	}
private:
	int x;
	int y;
	bool valid;
};

class YRect{
public:
	YRect()
		: x(0)
		, y(0)
		, w(0)
		, h(0)
	{ }
	YRect(int x, int y, int w, int h)
		: x(x)
		, y(y)
		, w(w)
		, h(h)
	{ }
	int X(){
		return this->x;
	}
	int Y(){
		return this->y;
	}
	int W(){
		return this->w;
	}
	int H(){
		return this->h;
	}
private:
	int x;
	int y;
	int w;
	int h;
};

std::vector<YRect> FindContours(YFrame<bool> &src)
{
	int width = src.width;
	int height = src.height;
	int key = 1;
	YFrame<int> map(width, height);
	std::vector<YRect> rects;
	rects.reserve(100);
	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			if (src.Get(x, y) == true){
				if (map.Get(x, y) == 0){
					map.Get(x, y) = key;
					std::deque<YValidatedPoint> neighbors;
					neighbors.push_back(YValidatedPoint(x, y));
					int x1 = x;
					int y1 = y;
					int x2 = x;
					int y2 = y;
					while (true) {
						if (neighbors.size() == 0){
							YRect rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
							rects.push_back(rect);
							key++;
							break;
						}
						auto current = neighbors.front();
						int current_x = current.X();
						int current_y = current.Y();
						if (current_x < x1){
							x1 = current_x;
						}
						if (x2 < current_x){
							x2 = current_x;
						}
						if (current_y < y1){
							y1 = current_y;
						}
						if (y2 < current_y){
							y2 = current_y;
						}

						neighbors.pop_front();

						std::vector<YValidatedPoint> candidates;
						candidates.reserve(9);
						candidates.push_back(YValidatedPoint(current_x - 1, current_y - 1, 0, width, 0, height));
						candidates.push_back(YValidatedPoint(current_x - 0, current_y - 1, 0, width, 0, height));
						candidates.push_back(YValidatedPoint(current_x + 1, current_y - 1, 0, width, 0, height));
						candidates.push_back(YValidatedPoint(current_x - 1, current_y - 0, 0, width, 0, height));
						//candidates.push_back(YValidatedPoint(current_x - 0, current_y - 0, 0, width, 0, height)); // center
						candidates.push_back(YValidatedPoint(current_x + 1, current_y - 0, 0, width, 0, height));
						candidates.push_back(YValidatedPoint(current_x - 1, current_y + 1, 0, width, 0, height));
						candidates.push_back(YValidatedPoint(current_x - 0, current_y + 1, 0, width, 0, height));
						candidates.push_back(YValidatedPoint(current_x + 1, current_y + 1, 0, width, 0, height));
						for (auto& candidate : candidates){
							if (candidate.IsValid() == true){
								int candidate_x = candidate.X();
								int candidate_y = candidate.Y();
								if (src.Get(candidate_x, candidate_y) == true){
									if (map.Get(candidate_x, candidate_y) == 0){
										map.Get(candidate_x, candidate_y) = key;
										neighbors.push_back(YValidatedPoint(candidate_x, candidate_y));
									}
								}
							}
						}
					}
				}
			}
		}
	}

#if 0
	printf("------------------------------------------------------\n");
	printf("------------------------------------------------------\n");
	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			printf("(%03d, %03d) = %d\n", x, y, map.data[y*width + x]);
		}
	}
#endif
	return rects;
}

int main()
{
	cv::Mat src;
	src = cv::imread("sample.png");
	cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);
	cv::threshold(src, src, 127, 255, cv::THRESH_BINARY);

	int width = src.cols;
	int height = src.rows;
	auto frame = YFrame<bool>(width, height);
	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			frame.Get(x, y) = src.data[y*width + x] == 0 ? false : true;
		}
	}

	///////////////////////////////////////////////////////////////////////
	
	auto rects = FindContours(frame);
	cv::Mat display;
	cv::cvtColor(src, display, cv::COLOR_GRAY2BGR);
	for (auto rect : rects){
		cv::rectangle(display, cv::Rect(rect.X(), rect.Y(), rect.W(), rect.H()), cv::Scalar(255, 0, 0), 1);
	}
	cv::imshow("display", display);
	cv::waitKey(0);
}
