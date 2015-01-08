/*

Copyright (c) 2013, Tony Baltovski
All rights reserved.E2

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * Neither the name of  nor the names of its contributors may be used to
   endorse or promote products derived from this software without specific
   prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/
#include <math.h>
#include "ros_arduino_base/ros_arduino_base.h"

ROSArduinoBase::ROSArduinoBase(ros::NodeHandle nh, ros::NodeHandle nh_private):
  nh_(nh),
  nh_private_(nh_private),
  x_(0.0),
  y_(0.0),
  theta_(0.0),
  dx_(0.0),
  dy_(0.0),
  dtheta_(0.0),
  E1_counts_(0),
  E2_counts_(0),
  E3_counts_(0),
  old_E1_counts_(0),
  old_E2_counts_(0),
  old_E3_counts_(0)
{
  cmd_diff_vel_pub_ = nh_.advertise<ros_arduino_msgs::CmdDiffVel>("cmd_diff_vel", 5);
  odom_pub_ = nh_.advertise<nav_msgs::Odometry>("odom", 5);
  encoders_sub_ = nh_.subscribe("encoders", 5, &ROSArduinoBase::encodersCallback, this,
                                    ros::TransportHints().unreliable().reliable().tcpNoDelay());
  cmd_vel_sub_ = nh_.subscribe("cmd_vel", 5, &ROSArduinoBase::cmdVelCallback, this);
  update_gains_client_ = nh.serviceClient<ros_arduino_base::UpdateGains>("update_gains");
  dynamic_reconfigure::Server<ros_arduino_base::MotorGainsConfig>::CallbackType f = 
	                                  boost::bind(&ROSArduinoBase::motorGainsCallback, this, _1, _2);
  gain_server_.setCallback(f);
		
  // ROS driver params
  nh_private.param<double>("counts_per_rev", counts_per_rev_, 1024);
  nh_private.param<double>("gear_ratio", gear_ratio_, (75.0 / 1.0));
  nh_private.param<int>("encoder_on_motor_shaft", encoder_on_motor_shaft_, 0);
  nh_private.param<double>("wheel_radius", wheel_radius_, 0.0157925);  //[m]
  nh_private.param<double>("base_width", base_width_ , 0.225);

  if (encoder_on_motor_shaft_ == 1)
  {
    meters_per_counts_ = ((M_PI * 2 * wheel_radius_) / (counts_per_rev_ * gear_ratio_));
  }
  else
  {
    meters_per_counts_ = ((M_PI * 2 * wheel_radius_) / counts_per_rev_);
  }
}

ROSArduinoBase::~ROSArduinoBase()
{
  ROS_INFO("Destroying ROS Arduino Base");
}


void ROSArduinoBase::cmdVelCallback(const geometry_msgs::Twist::ConstPtr& vel_msg)
{
  ros_arduino_msgs::CmdDiffVel cmd_diff_vel_msg;
  // Convert to velocity to each wheel
  cmd_diff_vel_msg.right = vel_msg->linear.x; 	// (vel_msg->linear.x + ((base_width_ /  2) * vel_msg->angular.z));
  cmd_diff_vel_msg.left  = vel_msg->linear.x;	//(vel_msg->linear.x + ((base_width_ / -2) * vel_msg->angular.z));
  cmd_diff_vel_pub_.publish(cmd_diff_vel_msg);
}

void ROSArduinoBase::motorGainsCallback(ros_arduino_base::MotorGainsConfig &config, uint32_t level) 
{
  gains_[0] = config.K_P;
  gains_[1] = config.K_I;
  gains_[2] = config.K_D;
  ros_arduino_base::UpdateGains srv;
  srv.request.p = gains_[0];
  srv.request.i = gains_[1];
  srv.request.d = gains_[2];
  if (update_gains_client_.call(srv))
  {
    ROS_INFO("Motor Gains changed to P:%f I:%f D: %f",gains_[0],gains_[1],gains_[2]);
  }
  else
  {
    ROS_ERROR("Failed to update gains");
  }

}

void ROSArduinoBase::encodersCallback(const ros_arduino_msgs::Encoders::ConstPtr& encoders_msg)
{
  encoder_current_time_ = ros::Time::now();
  odom_broadcaster_ = new tf::TransformBroadcaster();
  nav_msgs::Odometry odom;
  E1_counts_ = encoders_msg->E1;
  E2_counts_ = encoders_msg->E2;
  E3_counts_ = encoders_msg->E3;

  double dt = (encoder_current_time_ - encoder_previous_time_).toSec();  // [s]
  double velocity_estimate_E1_ = meters_per_counts_ * (E1_counts_ - old_E1_counts_) / dt;  // [m/s]
  double velocity_estimate_E2_ = meters_per_counts_ * (E2_counts_ - old_E2_counts_) / dt;  // [m/s]
  double velocity_estimate_E3_ = meters_per_counts_ * (E3_counts_ - old_E3_counts_) / dt;  // [m/s]
  //double delta_s = meters_per_counts_ * (((right_counts_ - old_right_counts_)
  //                                        + (left_counts_ - old_left_counts_)) / 2.0);  // [m]
  //double delta_theta = meters_per_counts_ * (((right_counts_ - old_right_counts_)
  //                                        - (left_counts_ - old_left_counts_)) / base_width_);  // [radians]
  double delta_theta = 0.0;
  double dx = (velocity_estimate_E1_+velocity_estimate_E2_+velocity_estimate_E3_) / 3.0;  // [m]
  //double dy = delta_s * sin(theta_ + delta_theta / 2.0);  // [m]
  x_ += dx;  // [m]
  //y_ += dy;  // [m]
  theta_ += delta_theta;  // [radians]
  geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(theta_);

  geometry_msgs::TransformStamped odom_trans;
  odom_trans.header.stamp = encoder_current_time_;
  odom_trans.header.frame_id = "odom";
  odom_trans.child_frame_id = "base_footprint";
  odom_trans.transform.translation.x = x_;
  odom_trans.transform.translation.y = 0.0;
  odom_trans.transform.translation.z = 0.0;
  odom_trans.transform.rotation = odom_quat;
  odom_broadcaster_->sendTransform(odom_trans);

  odom.header.stamp = encoder_current_time_;
  odom.header.frame_id = "odom";
  odom.child_frame_id = "base_footprint";
  odom.pose.pose.position.x = x_;
  odom.pose.pose.position.y = 0.0;
  odom.pose.pose.position.z = 0.0;
  odom.pose.pose.orientation = odom_quat;
  odom.twist.twist.linear.x = dx / dt;
  odom.twist.twist.linear.y = 0.0;
  odom.twist.twist.linear.z = 0.0;
  odom.twist.twist.angular.x = 0.0;
  odom.twist.twist.angular.y = 0.0;
  odom.twist.twist.angular.z = 0.0;
  // Fill in the covar. TODO make a param
  odom.pose.covariance[0]  = 0.01;  // x
  odom.pose.covariance[7]  = 99999;  // y
  odom.pose.covariance[14] = 99999;  // z
  odom.pose.covariance[21] = 99999;  // roll
  odom.pose.covariance[28] = 99999;  // pitch
  odom.pose.covariance[35] = 99999;  // yaw(theta)
  odom.twist.covariance[0]  = 0.01;  // x
  odom.twist.covariance[7]  = 99999;  // y
  odom.twist.covariance[14] = 99999;  // z
  odom.twist.covariance[21] = 99999;  // roll
  odom.twist.covariance[28] = 99999;  // pitch
  odom.twist.covariance[35] = 99999;  // yaw(theta)

  odom_pub_.publish(odom);

  encoder_previous_time_ = encoder_current_time_;
  old_E1_counts_ = E1_counts_;
  old_E2_counts_ = E2_counts_;
  old_E3_counts_ = E3_counts_;
}
