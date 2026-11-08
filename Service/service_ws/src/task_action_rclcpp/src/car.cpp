#include"task_action_rclcpp/car.h"

float Car::move_step(){
    float dx =target_x_ - current_x_;
    float dy =target_y_ - current_y_;
    float distance = std::sqrt(dx*dx + dy*dy);

    if(distance< 0.01){
        current_x_ =target_x_;
        current_y_=target_y_;
        vx_=0.0;
        vy_=0.0;
        return 0.0;
    }
    float move_dist =std::min(step_size_ , distance);
    vx_ =(dx/distance) * move_dist;
    vy_ =(dy/distance) * move_dist;
    current_x_ += vx_;
    current_y_ += vy_;
    return move_dist;
}

bool Car::set_goal(float x , float y){
    target_x_ =x;
    target_y_ =y;
    cancel_flag_ =false;

    if(close_goal()){
        vx_=0.0;
        vy_=0.0;
        return false;
    }
    return true;
}

void Car::stop_move(){
    cancel_flag_=true;
}

geometry_msgs::msg::Point Car::get_current_position(){
    geometry_msgs::msg::Point p;
    p.x = current_x_;
    p.y = current_y_;
    return p;
}

geometry_msgs::msg::Vector3 Car::get_current_velocity(){
    geometry_msgs::msg::Vector3 v;
    v.x = vx_;
    v.y = vy_;
    return v;
}

float Car::get_distance_to_target(){
    float dx = target_x_ -current_x_;
    float dy = target_y_ -current_y_;
    return std::sqrt(dx*dx + dy*dy);
}

float Car::get_total_times(){
    return total_times;
}

void Car::add_time(float time) { total_times += time; }

bool Car::close_goal() {
    return get_distance_to_target() < 0.01;
}

void Car::set_step_size(float size) {
    step_size_ = size;
}