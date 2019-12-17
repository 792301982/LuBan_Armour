#ifndef ARMOR
#define ARMOR

/*
*   This class describes the info of lights, including angle level, width, length, score
*/
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

#endif // ARMOR

