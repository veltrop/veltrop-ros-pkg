//#include <opencv/cv.h>
#include <iostream>
#include <opencv/cvwimage.h>
#include <opencv/highgui.h>
#include <ros/ros.h>
#include <cv_bridge/CvBridge.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/SetCameraInfo.h>
#include <image_transport/image_transport.h>
#include <veltrobot_sensors/PairYUYVImages.h>

namespace veltrobot_sensors
{

// YUYV to RGB conversion
// There's many methods to this, and much ado about the color range...
// The below method nice because its only integer.
#define SAT(c) if (c & (~255)) { if (c < 0) c = 0; else c = 255; }
static void
//yuyv16_to_bgr24(int width, int height, const unsigned char *src, unsigned char *dst)
yuyv16_to_rgb24(int width, int height, const unsigned char *src, unsigned char *dst)
{
   unsigned char *s;
   unsigned char *d;
   int l, c;
   int r, g, b, cr, cg, cb, y1, y2;

   l = height;
   s = (unsigned char*)src;
   d = dst;
   while (l--)
   {
      c = width >> 1;
      while (c--)
      {
         y1 = *s++;
         cb = ((*s - 128) * 454) >> 8;
         cg = (*s++ - 128) * 88;
         y2 = *s++;
         cr = ((*s - 128) * 359) >> 8;
         cg = (cg + (*s++ - 128) * 183) >> 8;

         r = y1 + cr;
         b = y1 + cb;
         g = y1 - cg;
         SAT(r);
         SAT(g);
         SAT(b);

         //*d++ = b;
         //*d++ = g;
         //*d++ = r;
         *d++ = r;
         *d++ = g;
         *d++ = b;
        
         r = y2 + cr;
         b = y2 + cb;
         g = y2 - cg;
         SAT(r);
         SAT(g);
         SAT(b);

         //*d++ = b;
         //*d++ = g;
         //*d++ = r;
         *d++ = r;
         *d++ = g;
         *d++ = b;
      }
   }
}

class RepublishStereo
{
public:
  RepublishStereo()
  : n_()
  , it_(n_)
  , left_ipl_(NULL)
  , right_ipl_(NULL)
  {
    ros::NodeHandle n_private("~"); 
    std::string input_stereo_name, output_stereo_name, left_frame_id, right_frame_id;
    
    n_private.param<std::string>("input_stereo_name", input_stereo_name, "/stereo_yuyv");
    n_private.param<std::string>("output_stereo_name", output_stereo_name, "/stereo");
    n_private.param<std::string>("left_frame_id", left_frame_id, "stereo_camera");
    n_private.param<std::string>("right_frame_id", right_frame_id, "stereo_camera");
  
    {
      std::string calibration;
      n_private.param<std::string>("calibration", calibration, ""); 
      if (calibration == "")
        n_.param<std::string>("calibration", calibration, ""); 
      
      std::cout << calibration;
      
      std::istringstream calibration_stream(calibration); 
      char line[512];
      
      // left
    
      for (int i=0; i < 14; i++) 
      {
        calibration_stream.getline(line,512);
        std::cout << i << " : " << line << std::endl;
      }
      
      for (int i=0; i < 9; i++) 
      {
        calibration_stream >> left_info_.K[i];
        std::cout << i << " : " << left_info_.K[i] << std::endl;
      }
      
      for (int i=0; i < 3; i++) 
      {
        calibration_stream.getline(line,512);
        std::cout << i << " : " << line << std::endl;
      }
      
      for (int i=0; i < 5; i++) 
      {
        calibration_stream >> left_info_.D[i];
        std::cout << i << " : " << left_info_.D[i] << std::endl;
      }
      
      for (int i=0; i < 3; i++) 
      {
        calibration_stream.getline(line,512);
        std::cout << i << " : " << line << std::endl;
      }

      for (int i=0; i < 9; i++) 
      {
        calibration_stream >> left_info_.R[i];
        std::cout << i << " : " << left_info_.R[i] << std::endl;
      }
      
      for (int i=0; i < 3; i++) 
      {
        calibration_stream.getline(line,512);
        std::cout << i << " : " << line << std::endl;
      }
      
      for (int i=0; i < 12; i++) 
      {
        calibration_stream >> left_info_.P[i];
        std::cout << i << " : " << left_info_.P[i] << std::endl;
      }
      
      // right
      
      for (int i=0; i < 16; i++) 
      {
        calibration_stream.getline(line,512);
        std::cout << i << " : " << line << std::endl;
      }
      
      for (int i=0; i < 9; i++) 
      {
        calibration_stream >> right_info_.K[i];
        std::cout << i << " : " << right_info_.K[i] << std::endl;
      }
      
      for (int i=0; i < 3; i++) 
      {
        calibration_stream.getline(line,512);
        std::cout << i << " : " << line << std::endl;
      }
      
      for (int i=0; i < 5; i++) 
      {
        calibration_stream >> right_info_.D[i];
        std::cout << i << " : " << right_info_.D[i] << std::endl;
      }
      
      for (int i=0; i < 3; i++) 
      {
        calibration_stream.getline(line,512);
        std::cout << i << " : " << line << std::endl;
      }
      
      for (int i=0; i < 9; i++) 
      {
        calibration_stream >> right_info_.R[i];
        std::cout << i << " : " << right_info_.R[i] << std::endl;
      }
      
      for (int i=0; i < 3; i++) 
      {
        calibration_stream.getline(line,512);
        std::cout << i << " : " << line << std::endl;
      }
      
      for (int i=0; i < 12; i++) 
      {
        calibration_stream >> right_info_.P[i];
        std::cout << i << " : " << right_info_.P[i] << std::endl;
      }
      
      /*
      # oST version 5.0 parameters
      
      
      [image]
      
      width
      640
      
      height
      480
      
      [narrow_stereo/left]
      
      camera matrix
      406.611484 0.000000 339.139341
      0.000000 406.967314 196.598798
      0.000000 0.000000 1.000000
      
      distortion
      -0.340734 0.099348 0.000478 -0.001282 0.0000
      
      rectification
      0.999491 0.004796 -0.031551
      -0.005297 0.999861 -0.015818
      0.031471 0.015977 0.999377
      
      projection
      391.809437 0.000000 350.758442 0.000000
      0.000000 391.809437 216.236387 0.000000
      0.000000 0.000000 1.000000 0.000000
      
      # oST version 5.0 parameters
      
      
      [image]
      
      width
      640
      
      height
      480
      
      [narrow_stereo/right]
      
      camera matrix
      409.483821 0.000000 314.805491
      0.000000 411.514333 226.586651
      0.000000 0.000000 1.000000
      
      distortion
      -0.354106 0.112187 0.001856 -0.001731 0.0000
      
      rectification
      0.998286 -0.002946 -0.058442
      0.003876 0.999868 0.015793
      0.058388 -0.015993 0.998166
      
      projection
      391.809437 0.000000 350.758442 -13.078337
      0.000000 391.809437 216.236387 0.000000
      0.000000 0.000000 1.000000 0.000000
      */
    }
    
    left_info_.header.frame_id = left_frame_id;
    right_info_.header.frame_id = right_frame_id;
    
    std::string left_image_str = output_stereo_name + "/left/image_raw";
    std::string right_image_str = output_stereo_name + "/right/image_raw";
    std::string left_info_str = output_stereo_name + "/left/camera_info";
    std::string right_info_str = output_stereo_name + "/right/camera_info";
    std::string set_left_info_str = output_stereo_name + "/left/set_camera_info";
    std::string set_right_info_str = output_stereo_name + "/right/set_camera_info";
    
    left_info_srv_ = n_.advertiseService(set_left_info_str, &RepublishStereo::setLeftCameraInfoCB, this);
    right_info_srv_ = n_.advertiseService(set_right_info_str, &RepublishStereo::setRightCameraInfoCB, this);

    left_image_pub_ = it_.advertise(left_image_str.c_str(), 1);
    right_image_pub_ = it_.advertise(right_image_str.c_str(), 1);
    
    left_info_pub_ = n_.advertise<sensor_msgs::CameraInfo>(left_info_str.c_str(), 1);
    right_info_pub_ = n_.advertise<sensor_msgs::CameraInfo>(right_info_str.c_str(), 1);
    
    stereo_sub_ = n_.subscribe(input_stereo_name, 1, &RepublishStereo::getStereoCB, this);
  }
  
  ~RepublishStereo()
  {
    cvReleaseImage(&left_ipl_);
    cvReleaseImage(&right_ipl_);
  }
  
  void spin()
  {
    t_prev_ = ros::Time::now();
    frames_ = 0;
    ros::spin();
  }

private:
  ros::NodeHandle n_; 
  ros::Subscriber stereo_sub_;
  ros::Time t_prev_;
  int frames_;
  image_transport::ImageTransport it_;
  image_transport::Publisher left_image_pub_;
  image_transport::Publisher right_image_pub_;
  ros::Publisher left_info_pub_;
  ros::Publisher right_info_pub_;
  IplImage *left_ipl_, *right_ipl_;
  sensor_msgs::CameraInfo left_info_;
  sensor_msgs::CameraInfo right_info_;
  ros::ServiceServer left_info_srv_;
  ros::ServiceServer right_info_srv_; 
    
  void doFps()
  {
    if (++frames_ == 10)
    {
      frames_ = 0;
      ros::Duration d = ros::Time::now() - t_prev_;
      t_prev_ = ros::Time::now();
      float fps = 10.0f / d.toSec();
      ROS_INFO("last 10 image pairs received at %f fps\n", fps);
    }  
  }
  
  bool setLeftCameraInfoCB(sensor_msgs::SetCameraInfo::Request& request,
                           sensor_msgs::SetCameraInfo::Response& response)
  {
    left_info_.roi = request.camera_info.roi;
    left_info_.D = request.camera_info.D;
    left_info_.K = request.camera_info.K;
    left_info_.R = request.camera_info.R;
    left_info_.P = request.camera_info.P;
    response.success = true;
    return true;
  }
  bool setRightCameraInfoCB(sensor_msgs::SetCameraInfo::Request& request,
                           sensor_msgs::SetCameraInfo::Response& response)
  {
    right_info_.roi = request.camera_info.roi;
    right_info_.D = request.camera_info.D;
    right_info_.K = request.camera_info.K;
    right_info_.R = request.camera_info.R;
    right_info_.P = request.camera_info.P;
    response.success = true;
    return true;
  }
  
  void getStereoCB(const veltrobot_sensors::PairYUYVImagesConstPtr& msg)
  {
    ros::Duration d = ros::Time::now() - msg->header.stamp;
    ROS_INFO("image pair receive delay %f\n", d.toSec());
    doFps();
        
    if (!left_ipl_)
      left_ipl_=cvCreateImage(cvSize(msg->left_image.width,msg->left_image.height), 8, 3);
    if (!right_ipl_)
      right_ipl_=cvCreateImage(cvSize(msg->left_image.width,msg->left_image.height), 8, 3);
    
    // if new image is diff size, shoud release old and make new. but that shouldn't happen...
        
    //yuyv16_to_bgr24(msg->left_image.width, msg->left_image.height,
    //                &msg->left_image.data[0], (unsigned char*)left_ipl_->imageData);        
    //yuyv16_to_bgr24(msg->right_image.width, msg->right_image.height,
    //                &msg->right_image.data[0], (unsigned char*)right_ipl_->imageData);    
  
    //cvShowImage("left", left_ipl_);
    //cvShowImage("right", right_ipl_);
    
    yuyv16_to_rgb24(msg->left_image.width, msg->left_image.height,
                    &msg->left_image.data[0], (unsigned char*)left_ipl_->imageData);        
    yuyv16_to_rgb24(msg->right_image.width, msg->right_image.height,
                    &msg->right_image.data[0], (unsigned char*)right_ipl_->imageData); 
    
    /*IplImage *t;
    t = cvCloneImage(left_ipl_);
    cvSmooth(t, left_ipl_, CV_GAUSSIAN, 3, 3, 0, 0);
    cvReleaseImage(&t);
    t = cvCloneImage(right_ipl_);
    cvSmooth(t, right_ipl_, CV_GAUSSIAN, 3, 3, 0, 0);
    cvReleaseImage(&t);*/
    
    
    sensor_msgs::Image::Ptr left_image, right_image;
    //left_image = sensor_msgs::CvBridge::cvToImgMsg(left_ipl_, "bgr8");
    //right_image = sensor_msgs::CvBridge::cvToImgMsg(right_ipl_, "bgr8");
    left_image = sensor_msgs::CvBridge::cvToImgMsg(left_ipl_);
    right_image = sensor_msgs::CvBridge::cvToImgMsg(right_ipl_);
    
    left_info_.header.stamp = right_info_.header.stamp = msg->header.stamp;
    left_info_.header.seq = right_info_.header.seq = msg->header.seq;
    left_info_.height = right_info_.height = msg->left_image.height;
    left_info_.width = right_info_.width = msg->left_image.width;
    

    left_image->header.frame_id = left_info_.header.frame_id;
    right_image->header.frame_id = right_info_.header.frame_id;
    left_image->header.stamp = right_image->header.stamp = msg->header.stamp;
    left_image->header.seq = right_image->header.seq = msg->header.seq;
    left_image->encoding = right_image->encoding = sensor_msgs::image_encodings::RGB8;
    left_image->height = right_image->height = msg->left_image.height;
    left_image->width = right_image->width = msg->left_image.width;
    left_image->step = right_image->step = 3 * msg->left_image.width;    
    
    left_image_pub_.publish(left_image);
    left_info_pub_.publish(left_info_);
    
    right_image_pub_.publish(right_image);
    right_info_pub_.publish(right_info_);
  }
};

} // namespace veltrobot_sensors

int main(int argc, char** argv)
{
  ros::init(argc, argv, "republish_stereo");
  cvInitSystem(argc, argv);
  //cvStartWindowThread();
  
  //cvNamedWindow("left");
  //cvNamedWindow("right");
  
  veltrobot_sensors::RepublishStereo this_node;
  this_node.spin();
  
  return 0;
}
