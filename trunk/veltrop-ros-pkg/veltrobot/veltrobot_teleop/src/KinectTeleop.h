#ifndef TELEOP_KINECT_H_
#define TELEOP_KINECT_H_

#include "KinectController.h"
#include <veltrobot_msgs/EnableJointGroup.h>
#include <string>

using std::string;

namespace veltrobot_teleop
{

class KinectTeleop
{
	public:
		KinectTeleop();
		
		void init();
		void publishTransform(KinectController& kinect_controller,
		                      XnUserID const& user, XnSkeletonJoint const& joint,
													string const& frame_id, string const& child_frame_id);
		void processKinect(KinectController& kinect_controller);

	private:
		void enableJointGroupCB(const veltrobot_msgs::EnableJointGroupConstPtr& msg);
    
    ros::Publisher   left_arm_destination_pub_;
    ros::Publisher   right_arm_destination_pub_;
		ros::Publisher   motion_pub_;
		ros::Publisher   joint_states_pub_;
		ros::Publisher   cmd_vel_pub_;
    ros::Subscriber  enable_joint_group_sub_;
		bool             publish_kinect_tf_;
    bool             right_arm_enabled_, left_arm_enabled_; 
    bool             legs_enabled_, motion_enabled_;
};

} // namespace veltrobot_teleop

#endif
