#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "lr_interfaces/action/move_car.hpp"
//客户
class CarControlNode : public rclcpp::Node{
public:

    CarControlNode(std::string name) : Node(name) {
        RCLCPP_INFO(this->get_logger(),"节点已启动：%s.",name.c_str());
        this->declare_parameter("goal_x", 10.0);
        this->declare_parameter("goal_y", 10.0);

        this->client_ptr_ = rclcpp_action::create_client<lr_interfaces::action::MoveCar>(this,"move_car");
        this->timer_ = create_wall_timer(std::chrono::milliseconds(500),std::bind(&CarControlNode::send_goal,this));
    }

    void send_goal(){

        this->timer_->cancel();

        if(!this->client_ptr_->wait_for_action_server(std::chrono::seconds(10))) {
            RCLCPP_ERROR(this->get_logger(),"Action server not available after waiting");
            rclcpp::shutdown();
            return;
        }
        auto goal_msg =lr_interfaces::action::MoveCar::Goal();

        goal_msg.target_position.x = this->get_parameter("goal_x").as_double();
        goal_msg.target_position.y = this->get_parameter("goal_y").as_double();
        RCLCPP_INFO(this->get_logger(), "发送目标坐标: ( %f , %f )", goal_msg.target_position.x, goal_msg.target_position.y);

        auto send_goal_options = rclcpp_action::Client<lr_interfaces::action::MoveCar>::SendGoalOptions();
        //goal
        send_goal_options.goal_response_callback =std::bind(&CarControlNode::goal_response_callback, this, std::placeholders::_1);
        //feedback
        send_goal_options.feedback_callback =std::bind(&CarControlNode::feedback_callback, this, std::placeholders::_1, std::placeholders::_2);
        //result
        send_goal_options.result_callback =std::bind(&CarControlNode::result_callback, this, std::placeholders::_1);

        this->client_ptr_->async_send_goal(goal_msg, send_goal_options);
    }


//rclcpp_action::ClientGoalHandle<MoveCar>
private:
    rclcpp_action::Client<lr_interfaces::action::MoveCar>::SharedPtr client_ptr_;
    rclcpp::TimerBase::SharedPtr timer_;

    void goal_response_callback(rclcpp_action::ClientGoalHandle<lr_interfaces::action::MoveCar>::SharedPtr goal_handle){
        if(!goal_handle) RCLCPP_ERROR(this->get_logger(),"命令被拒绝");
        else RCLCPP_INFO(this->get_logger(),"命令已接收");
    }

    void feedback_callback(rclcpp_action::ClientGoalHandle<lr_interfaces::action::MoveCar>::SharedPtr , const std::shared_ptr<const lr_interfaces::action::MoveCar::Feedback> feedback){
        RCLCPP_INFO(this->get_logger(),"当前位置: ( %f , %f ),当前速度: ( %f , %f ),目标距离:%f",feedback->current_position.x,feedback->current_position.y,feedback->current_velocity.x,feedback->current_velocity.y,feedback->distance_to_target);
    }

    void result_callback(const rclcpp_action::ClientGoalHandle<lr_interfaces::action::MoveCar>::WrappedResult& result){
        switch(result.code) {
            case rclcpp_action::ResultCode::SUCCEEDED: break;
            case rclcpp_action::ResultCode::ABORTED: RCLCPP_ERROR(this->get_logger(),"Goal was aborted"); return;
            case rclcpp_action::ResultCode::CANCELED: RCLCPP_ERROR(this->get_logger(),"命令被取消，当前位置 (%f , %f )，距目标距离 : %f",result.result->final_position.x , result.result->final_position.y,result.result->remaining_distance); return;
            default: RCLCPP_ERROR(this->get_logger(),"Unknown result code"); return;
        }
        RCLCPP_INFO(this->get_logger(),"到达目的地 , 耗时: %f",result.result->total_time_sec);
    }
};

int main(int argc, char ** argv){
    rclcpp::init(argc,argv);
    auto action_client = std::make_shared<CarControlNode>("move_car");
    rclcpp::spin(action_client);
    rclcpp::shutdown();
    return 0;
}