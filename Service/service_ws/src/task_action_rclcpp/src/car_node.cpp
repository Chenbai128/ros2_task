#include "task_action_rclcpp/car.h"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "lr_interfaces/action/move_car.hpp"

class CarNode : public rclcpp::Node {
public:
    using MoveCar = lr_interfaces::action::MoveCar;
    using GoalHandleMoveCar = rclcpp_action::ServerGoalHandle<MoveCar>;

    CarNode(std::string name) : Node(name) {
        RCLCPP_INFO(this->get_logger(),"节点已启动：%s",name.c_str());


        this->declare_parameter("step_speed", 1.0);
        double speed = this->get_parameter("step_speed").as_double();
        car.set_step_size(speed);
        param_callback_handle_ = this->add_on_set_parameters_callback(std::bind(&CarNode::param_callback, this, std::placeholders::_1));

        this->action_server_ = rclcpp_action::create_server<MoveCar>(this,"move_car",
            std::bind(&CarNode::handle_goal , this,std::placeholders::_1,std::placeholders::_2),
            std::bind(&CarNode::handle_cancel , this,std::placeholders::_1),
            std::bind(&CarNode::handle_accepted, this,std::placeholders::_1) );
    }

private:
    Car car;
    rclcpp_action::Server<MoveCar>::SharedPtr action_server_;
    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_callback_handle_;

    rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID &uuid , std::shared_ptr<const MoveCar::Goal> goal){
        (void)uuid;
        RCLCPP_INFO(this->get_logger(), "收到目标坐标: ( %f , %f )", goal->target_position.x, goal->target_position.y);
         return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandleMoveCar> goal_handle){
        RCLCPP_INFO(this->get_logger(),"收到取消请求");
        car.stop_move();
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    void handle_accepted(const std::shared_ptr<GoalHandleMoveCar> goal_handle){
        std::thread{std::bind(&CarNode::execute_move,this,std::placeholders::_1),goal_handle}.detach();
    }
    rcl_interfaces::msg::SetParametersResult param_callback(const std::vector<rclcpp::Parameter> & parameters) {
        rcl_interfaces::msg::SetParametersResult result;
        result.successful = true;
        for (const auto & param : parameters) {
            if (param.get_name() == "step_speed") {
                double new_speed = param.as_double();
                car.set_step_size(new_speed);
                RCLCPP_INFO(this->get_logger(), "速度已更新为: %f", new_speed);
            }
        }
        return result;
    }

    void execute_move(const std::shared_ptr<GoalHandleMoveCar> goal_handle){
        const auto goal = goal_handle->get_goal();

        auto result =std::make_shared<MoveCar::Result>();
        auto feedback = std::make_shared<MoveCar::Feedback>();

        rclcpp::Rate rate = rclcpp::Rate(2);
        car.set_goal(goal->target_position.x , goal->target_position.y);

        while(rclcpp::ok() && !car.close_goal()){
            car.set_goal(goal->target_position.x , goal->target_position.y);

            car.move_step();
            car.add_time(0.5);
            feedback->current_position = car.get_current_position();
            feedback->current_velocity = car.get_current_velocity();
            feedback->distance_to_target = car.get_distance_to_target();
            goal_handle->publish_feedback(feedback);

            if(goal_handle->is_canceling()) {
                car.stop_move();

                result->final_position = car.get_current_position();
                result->remaining_distance = car.get_distance_to_target();
                goal_handle->canceled(result);
                RCLCPP_INFO(this->get_logger(),"任务取消");
                return;
            }
            RCLCPP_INFO(this->get_logger(),"返回当前状态");
            rate.sleep();
        }
        result->final_position = car.get_current_position();
        result->total_time_sec = car.get_total_times();
        goal_handle->succeed(result);
        RCLCPP_INFO(this->get_logger(),"到到达目的地");
    }
};

int main(int argc , char **argv){
    rclcpp::init(argc , argv);
    auto car_node = std::make_shared<CarNode>("move_car");
    rclcpp::spin(car_node);
    rclcpp::shutdown();
}

