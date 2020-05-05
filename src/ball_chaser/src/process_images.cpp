#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    
    ROS_INFO_STREAM("Requesting new linear x and angular z velocities");

    // Request linear_x and angular_z
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the ball_chaser command_robot service and pass linear and angular params
    if (!client.call(srv))
        ROS_ERROR("Failed to call service safe_move");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

  int white_pixel = 255;
  bool found_ball=false;
  int ball_x,ball_y;
  int bpp=img.step/img.width;
  float lin_x,ang_z;

  // TODO: Loop through each pixel in the image and check if there's a bright white one
  // Then, identify if this pixel falls in the left, mid, or right side of the image
  // Depending on the white ball position, call the drive_bot function and pass velocities to it
  // Request a stop when there's no white ball seen by the camera
  
  found_ball=false;
  for (int y = 0; y < img.height; y++) {
    for(int x=0; x < img.width; x++) {
      if (img.data[y*img.step+x*bpp] == white_pixel) {
        found_ball=true;
        ball_x=x;ball_y=y;
        break;
      }
    }
  }
  
  if(found_ball) {
    ROS_INFO_STREAM("ball_x: " + std::to_string(ball_x));
    if(ball_x < 266)
    {
      ROS_INFO_STREAM("ball in the left 1/3");
      lin_x=0.1;
      ang_z=0.01*(400-ball_x);
    } 
    else
      if(ball_x >= 266 && ball_x < 533)
      {
        ROS_INFO_STREAM("ball in the middle section");
        lin_x=0.5;
        if(ball_x<400)
          ang_z=0.01*(400-ball_x);
        else
          ang_z=-0.01*(ball_x-400);
        
      }
      else
        if(ball_x >= 533) 
        {
          ROS_INFO_STREAM("ball in the right section");
          lin_x=0.1;
          ang_z=-0.01*(ball_x-400);
        }
    drive_robot(lin_x, ang_z);
  } else drive_robot(0, -1.5);
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}