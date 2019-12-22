#ifndef ARMORDETECTOR_H
#define ARMORDETECTOR_H
#endif // ARMORDETECTOR_H
#include <opencv4/opencv2/opencv.hpp>
enum ObjectType
{
    UNKNOWN_ARMOR = 0,
    SMALL_ARMOR = 1,
    BIG_ARMOR = 2,
    MINI_RUNE = 3,
    GREAT_RUNE = 4
};
enum ColorChannels
{
    BLUE = 0,
    GREEN = 1,
    RED = 2
};
/**************************************************************
 * DEBUG_PRETREATMENT 	只显示简单预处理后的图像
 * DEBUG_DETECTION	记录以下信息：
 * 				    	// 投资回报率区域 						: yellow
 * 					// 所有可能的灯条				: magenta
 * 					// 可能的光对(DEBUG_PAIR overlapped)	: cyan
 *					// 可能装甲区域的顶点			: white
 * 					// 结果装甲区顶点			: green
 * SHOW_RESULT		只显示检测结果（显示装甲顶点）		: green
 * GET_ARMOR_PIC	采集装甲区域样本
 *
 * Notice:		1. 所有宏定义都可以单独使用
 * 			2. 如果您想关注处理的特定部分，我建议您注释.cpp中调试检测的所有无关部分。或者您可以重写调试交互模式。
 * 			3. 如果使用“获取防具图片”，请记住更改图片的路径
 **************************************************************/
//#define DEBUG_PRETREATMENT
//#define DEBUG_DETECTION
#define SHOW_RESULT
//#define GET_ARMOR_PIC


/*
*	此结构存储装甲检测的所有参数（不包括svm）
*/
struct ArmorParam
{
    //Pre-treatment 预处理
    int brightness_threshold;
    int color_threshold;
    float light_color_detect_extend_ratio;

    //Filter lights 滤光灯
    float light_min_area;
    float light_max_angle;
    float light_min_size;
    float light_contour_min_solidity;
    float light_max_ratio;

    //Filter pairs 滤波器对
    float light_max_angle_diff_;
    float light_max_height_diff_ratio_; // hdiff / max(r.length, l.length)
    float light_max_y_diff_ratio_;  // ydiff / max(r.length, l.length)
    float light_min_x_diff_ratio_;

    //Filter armor 臂式过滤器
    float armor_big_armor_ratio;
    float armor_small_armor_ratio;
    float armor_min_aspect_ratio_;
    float armor_max_aspect_ratio_;

    //other params  其他参数
    float sight_offset_normalized_base;
    float area_normalized_base;
    int enemy_color;
    int max_track_num = 3000;

    /*
    *	@Brief: 为各项参数赋默认值
    */
    ArmorParam()
    {
        //pre-treatment 预处理
        brightness_threshold = 210;
        color_threshold = 40;
        light_color_detect_extend_ratio = 1.1;

        // Filter lights滤光灯
        light_min_area = 10;
        light_max_angle = 45.0;
        light_min_size = 5.0;
        light_contour_min_solidity = 0.5;
        light_max_ratio = 1.0;

        // Filter pairs 滤光灯
        light_max_angle_diff_ = 7.0; //20
        light_max_height_diff_ratio_ = 0.2; //0.5
        light_max_y_diff_ratio_ = 2.0; //100
        light_min_x_diff_ratio_ = 0.5; //100

        // Filter armor 臂式过滤器
        armor_big_armor_ratio = 3.2;
        armor_small_armor_ratio = 2;
        //armor_max_height_ = 100.0;
        //armor_max_angle_ = 30.0;
        armor_min_aspect_ratio_ = 1.0;
        armor_max_aspect_ratio_ = 5.0;

        //other params  其他参数
        //ized_base = 200;
        area_normalized_base = 1000;
        enemy_color = BLUE;
    }
};
class LightDescriptor
{
public:
    LightDescriptor() {};
    LightDescriptor(const cv::RotatedRect& light)
    {
        width = light.size.width;
        length = light.size.height;
        center = light.center;
        angle = light.angle;
        area = light.size.area();
    }
    const LightDescriptor& operator =(const LightDescriptor& ld)
    {
        this->width = ld.width;
        this->length = ld.length;
        this->center = ld.center;
        this->angle = ld.angle;
        this->area = ld.area;
        return *this;
    }

    /*
    *	@Brief: return the light as a cv::RotatedRect object
    *   将灯光作为cv：：rotatedect对象返回
    */
    cv::RotatedRect rec() const
    {
        return cv::RotatedRect(center, cv::Size2f(width, length), angle);
    }

public:
    float width;
    float length;
    cv::Point2f center;
    float angle;
    float area;
};

class ArmorDescriptor
{
    public:
    ArmorDescriptor();
    ArmorDescriptor(const LightDescriptor& lLight, const LightDescriptor& rLight, const int armorType, const cv::Mat& srcImg, const float rotationScore, ArmorParam param);
    void clear()
    {
        rotationScore = 0;
        sizeScore = 0;
        distScore = 0;
        finalScore = 0;
        for(int i = 0; i < 4; i++)
        {
            vertex[i] = cv::Point2f(0, 0);
        }
        type = UNKNOWN_ARMOR;
    }
    void getFrontImg(const cv::Mat& grayImg);
    bool isArmorPattern() const;
public:
    std::array<cv::RotatedRect, 2> lightPairs; //0 left, 1 right
    float sizeScore;		//S1 = e^(size)
    float distScore;		//S2 = e^(-offset)
    float rotationScore;		//S3 = -(ratio^2 + yDiff^2)
    float finalScore;

    std::vector<cv::Point2f> vertex; //four vertex of armor area, lihgt bar area exclued!!
        cv::Mat frontImg; //front img after prespective transformation from vertex,1 channel gray img

    //	0 -> small
    //	1 -> big
    //	-1 -> unkown
    int type;
};


