#ifndef TASK_ACTION_RCLCPP_CAR_H_
#define TASK_ACTION_RCLCPP_CAR_H_

#include "rclcpp/rclcpp.hpp"
#include "lr_interfaces/action/move_car.hpp"
#include "geometry_msgs/msg/point.hpp"
#include <atomic>
#include <cmath>

class Car{
public:
    Car()=default;
    ~Car()=default;
    float move_step();
    bool set_goal(float x,float y);

    geometry_msgs::msg::Point get_current_position();
    geometry_msgs::msg::Vector3 get_current_velocity();
    float get_distance_to_target();
    float get_total_times();
    void add_time(float time);
    void stop_move();
    bool close_goal();
    void set_step_size(float size);

private:
    float current_x_ = 0.0;
    float current_y_ = 0.0;
    float target_x_ = 0.0;
    float target_y_ = 0.0;
    
    float step_size_ = 1.0;
    float vx_ = 0.0;
    float vy_ = 0.0;

    float total_times=0.0;
    
    std::atomic<bool> cancel_flag_{false};
};
#endif