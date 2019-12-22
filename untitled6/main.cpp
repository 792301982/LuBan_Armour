#include <opencv4/opencv2/opencv.hpp>
#include <iostream>
#include<opencv4/opencv2/imgcodecs.hpp>
#include<opencv4/opencv2/highgui.hpp>
#include<opencv4/opencv2/highgui/highgui.hpp>
#include<opencv4/opencv2/imgproc.hpp>
#include<opencv4/opencv2/imgproc/imgproc.hpp>
#include<opencv4/opencv2/photo.hpp>
#include<opencv4/opencv2/core.hpp>
#include<opencv4/opencv2/core/core.hpp>

#include"opencv_extended.h"
#include"armordetector.h"
using namespace cv;
using namespace std;

#define CV_RETR_EXTERNAL 0
#define CV_RETR_LIST 1
#define CV_RETR_CCOMP 2
#define CV_RETR_TREE 3
#define CV_CHAIN_CODE 0
#define CV_CHAIN_APPROX_NONE 1
#define CV_CHAIN_APPROX_SIMPLE 2
#define CV_CHAIN_APPROX_TC89_L1 3
#define CV_CHAIN_APPROX_TC89_KCOS 4
#define CV_LINK_RUNS 5
#define BLUE 1
#define RED 0
#define ARMOR_NO 0

enum
{
    WIDTH_GREATER_THAN_HEIGHT,
    ANGLE_TO_UP
};

/*
*	@Brief:		// regulate the rotated rect
*	@Input:		// rotated rec
*				// regulation mode
*	@Return:	// regulated rec
*/
ArmorDescriptor::ArmorDescriptor(const LightDescriptor & lLight, const LightDescriptor & rLight, const int armorType, const cv::Mat & grayImg, float rotaScore, ArmorParam _param)
{
    //handle two lights
    lightPairs[0] = lLight.rec();
    lightPairs[1] = rLight.rec();

    cv::Size exLSize(int(lightPairs[0].size.width), int(lightPairs[0].size.height * 2));
    cv::Size exRSize(int(lightPairs[1].size.width), int(lightPairs[1].size.height * 2));
    cv::RotatedRect exLLight(lightPairs[0].center, exLSize, lightPairs[0].angle);
    cv::RotatedRect exRLight(lightPairs[1].center, exRSize, lightPairs[1].angle);

    cv::Point2f pts_l[4];
    exLLight.points(pts_l);
    cv::Point2f upper_l = pts_l[2];
    cv::Point2f lower_l = pts_l[3];

    cv::Point2f pts_r[4];
    exRLight.points(pts_r);
    cv::Point2f upper_r = pts_r[1];
    cv::Point2f lower_r = pts_r[0];

    vertex.resize(4);
    vertex[0] = upper_l;
    vertex[1] = upper_r;
    vertex[2] = lower_r;
    vertex[3] = lower_l;

    //set armor type
    type = armorType;

    //get front view
    getFrontImg(grayImg);
    rotationScore = rotaScore;

    // calculate the size score
    float normalized_area = contourArea(vertex) / _param.area_normalized_base;
    sizeScore = exp(normalized_area);

    // calculate the distance score
    Point2f srcImgCenter(grayImg.cols / 2, grayImg.rows / 2);
    float sightOffset = cvex::distance(srcImgCenter, cvex::crossPointOf(array<Point2f, 2>{vertex[0],vertex[2]}, array<Point2f, 2>{vertex[1],vertex[3]}));
    distScore = exp(-sightOffset / _param.sight_offset_normalized_base);

}
void ArmorDescriptor::getFrontImg(const Mat& grayImg)
{
    using cvex::distance;
    const Point2f&
        tl = vertex[0],
        tr = vertex[1],
        br = vertex[2],
        bl = vertex[3];

    int width, height;
    if(type == BIG_ARMOR)
    {
        width = 92;
        height = 50;
    }
    else
    {
        width = 50;
        height = 50;
    }

    Point2f src[4]{Vec2f(tl), Vec2f(tr), Vec2f(br), Vec2f(bl)};
    Point2f dst[4]{Point2f(0.0, 0.0), Point2f(width, 0.0), Point2f(width, height), Point2f(0.0, height)};
    const Mat perspMat = getPerspectiveTransform(src, dst);
    cv::warpPerspective(grayImg, frontImg, perspMat, Size(width, height));
}
cv::RotatedRect& adjustRec(cv::RotatedRect& rec, const int mode)
{
    using std::swap;
    float& width = rec.size.width;
    float& height = rec.size.height;
    float& angle = rec.angle;

    if (mode == WIDTH_GREATER_THAN_HEIGHT)
    {
        if (width < height)
        {
            swap(width, height);
            angle += 90.0;
        }
    }

    while (angle >= 90.0) angle -= 180.0;
    while (angle < -90.0) angle += 180.0;

    if (mode == ANGLE_TO_UP)
    {
        if (angle >= 45.0)
        {
            swap(width, height);
            angle -= 90.0;
        }
        else if (angle < -45.0)
        {
            swap(width, height);
            angle += 90.0;
        }
    }

    return rec;
}

cv::Mat _grayImg;

Mat shibie(cv::Mat &src ,cv::Mat &_roiImg,int &_flag) {
    // 图片路径换成本地的图片路径，注意是两个斜杠
    //Mat src = imread("C:\\Users\\79230\\Pictures\\zhuangjia.png");
    //请一定检查是否成功读图
    //if (src.empty())
    //{
    //cout << "Can not load image." << endl;
    //return 1;
    //}
    //imshow("input image", src);

    ///*
    //	将RGB颜色空间转到HSV颜色空间 灰度：CV_BGR2GRAY
    //*/
    //Mat hsvimg;
    //cvtColor(src, hsvimg, COLOR_BGR2HSV);
    //imshow("test", hsvimg);
    //waitKey(0);
    // 把一个3通道图像转换成3个单通道图像
#define _enemy_color 1
    vector<Mat> channels;
    std::vector<LightDescriptor> lightInfos;
    Mat _grayImg;
    split(src, channels);
    //预处理删除己方装甲板颜色
    if (_enemy_color == 0)
        _grayImg = channels.at(2) - channels.at(0);//Get red-blue image;
    else _grayImg = channels.at(0) - channels.at(2);//Get blue-red image;
    //imshow("2", channels.at(2));
    //imshow("0", channels.at(0));
    //imshow("预处理删除己方装甲板颜色", _grayImg);
    //waitKey(0);
    ///*
    //	二值化
    //*/
    //int LowH = 60;
    //int LowS = 120;
    //int LowV = 245;
    //int HighH = 120;
    //int HighS = 255;
    //int HighV = 255;
    //vector<Mat> hsvsplit;
    //split(hsvimg, hsvsplit);
    //equalizeHist(hsvsplit[2], hsvsplit[2]);

    //merge(hsvsplit, hsvimg);

    //Mat thresholdimg;
    //inRange(hsvimg, Scalar(LowH, LowS, LowV), Scalar(HighH, HighS, HighV), thresholdimg);
    ///*imshow("二值化后的图片", thresholdimg);
    //waitKey(0);*/


    Mat binBrightImg;
    //阈值化
    threshold(_grayImg, binBrightImg, 210
        , 255, cv::THRESH_BINARY);
    //imshow("阈值化", binBrightImg);
    //waitKey(0);
    //膨胀
    Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    dilate(binBrightImg, binBrightImg, element);
    //imshow("膨胀", binBrightImg);
    //waitKey(0);
    /*
        找轮廓
    */
    vector<vector<Point>> lightContours;
    findContours(binBrightImg.clone(), lightContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    //drawContours(src, lightContours,  -1, Scalar(0, 255, 0),5);
    //imshow("轮廓", src);
    //waitKey(0);

    for (const auto& contour : lightContours)
    {
        //得到面积
        float lightContourArea = contourArea(contour);
        //面积太小的不要
        if (contour.size() <= 5 || lightContourArea < 10) continue;
        //椭圆拟合区域得到外接矩形
        RotatedRect lightRec = fitEllipse(contour);//最小矩形
        //矫正灯条
        adjustRec(lightRec, ANGLE_TO_UP);
        //宽高比、凸度筛选灯条
        if (lightRec.size.width / lightRec.size.height >
            1.0 ||
            lightContourArea / lightRec.size.area() <
            0.5
            )continue;
        //对灯条范围适当扩大
        lightRec.size.width *= 1.1;
        lightRec.size.height *= 1.1;

        Rect lightRect = lightRec.boundingRect();
        const Rect srcBound(Point(0, 0), _roiImg.size());
        lightRect &= srcBound;
        Mat lightImg = _roiImg(lightRect);
        Mat lightMask = Mat::zeros(lightRect.size(), CV_8UC1);
        Point2f lightVertexArray[4];
        lightRec.points(lightVertexArray);
        std::vector<Point> lightVertex;
        for(int i = 0; i < 4; i++)
        {
            lightVertex.emplace_back(Point(lightVertexArray[i].x - lightRect.tl().x,
                                           lightVertexArray[i].y - lightRect.tl().y));
        }
        fillConvexPoly(lightMask, lightVertex, 255);//只能用来填充凸多边形
        //面积
        if(lightImg.size().area() <= 0 || lightMask.size().area() <= 0) continue;
        dilate(lightMask, lightMask, element);//膨化
        const Scalar meanVal = mean(lightImg, lightMask);
      if(((_enemy_color == BLUE) && (meanVal[BLUE] - meanVal[RED] > 20.0)) || (_enemy_color == RED && meanVal[RED] - meanVal[BLUE] > 20.0))
        {
             //因为颜色通道相减后己方灯条直接过滤，不需要判断颜色了,可以直接将灯条保存
            lightInfos.push_back(LightDescriptor(lightRec));
            //putText(debugColorImg, BGR, Point(lightVertexArray[0]) + _roi.tl(), FONT_HERSHEY_SIMPLEX, 0.4, cvex::GREEN, 1); //fontScalar 0.34
        }

    }
    //没找到灯条就返回没找到
    if(lightInfos.empty())
    {
         _flag = ARMOR_NO;
    }


    { //按灯条中心x从小到大排序
    sort(lightInfos.begin(), lightInfos.end(), [](const LightDescriptor& ld1, const LightDescriptor& ld2)
    {//Lambda函数,作为sort的cmp函数
        return ld1.center.x < ld2.center.x;
    });
    vector<int> minRightIndices(lightInfos.size(), -1);
    for(size_t i = 0; i < lightInfos.size(); i++)
    {//遍历所有灯条进行匹配
        for(size_t j = i + 1; (j < lightInfos.size()); j++)
        {
            const LightDescriptor& leftLight  = lightInfos[i];
            const LightDescriptor& rightLight = lightInfos[j];

           /*
            在2-3米的情况下工作
            形态相似/平行/高度相似
            */
            //角差
        float angleDiff_ = abs(leftLight.angle - rightLight.angle);
         //长度差比率
        float LenDiff_ratio = abs(leftLight.length - rightLight.length) / max(leftLight.length, rightLight.length);
           //筛选
        if(angleDiff_ >7.0 ||
           LenDiff_ratio >0.2)
        {
            continue;
        }
        /*
      适当位置：/灯条的y值足够接近
      长宽比合适
       */
        //左右灯条相距距离
        float dis = cvex::distance(leftLight.center, rightLight.center);
        //左右灯条长度的平均值
        float meanLen = (leftLight.length + rightLight.length) / 2;
        //左右灯条中心点y的差值
        float yDiff = abs(leftLight.center.y - rightLight.center.y);
         //y差比率
        float yDiff_ratio = yDiff / meanLen;
        //左右灯条中心点x的差值
        float xDiff = abs(leftLight.center.x - rightLight.center.x);
        //x差比率
        float xDiff_ratio = xDiff / meanLen;
        //相距距离与灯条长度比值
        float ratio = dis / meanLen;
        //筛选
        if(yDiff_ratio >2.0||
           xDiff_ratio < 0.5 ||
           ratio > 5.0 ||
           ratio < 1.0)
        {
            continue;
        }

        //按比值来确定大小装甲
       int armorType = ratio > 3.2 ? BIG_ARMOR : SMALL_ARMOR;//装甲板的比值对？
       if(armorType==2){//没有大装甲板了
           continue;
       }
       ArmorParam _param;
       std::vector<ArmorDescriptor> _armors;
       float ratiOff = max(float(2 - ratio), float(0));
       float yOff = yDiff / meanLen;
       float rotationScore = -(ratiOff * ratiOff + yOff * yOff);
       //得到匹配的装甲板
       ArmorDescriptor armor(leftLight, rightLight, armorType, channels.at(1), rotationScore, _param);
       _armors.emplace_back(armor);
       break;
    }
    }
}
    return src;
}

int main()
{
    VideoCapture capture(0);
    //    capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    //    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
    Mat frame;
    Mat _roiImg;
    int _flag;
    time_t start, end;
    int n=0;
    //capture.set(CV_CAP_PROP_FPS, 330);//fps
    time(&start);
    while (capture.read(frame))
    {
        n++;
        if(n%120==0)
        {
            time(&end);
            double seconds = difftime (end, start);
            double fps  = n / seconds;
            cout << "Estimated frames per second : " << fps << endl;
        }
        frame = shibie(frame,_roiImg,_flag);
        imshow("video-demo", frame);
        char c = waitKey(1);
        if (c == 27)
        {
            break;
        }
    }
    return 0;
}
