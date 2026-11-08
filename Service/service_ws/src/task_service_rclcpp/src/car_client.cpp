#include "rclcpp/rclcpp.hpp"
#include "lr_interfaces/srv/move.hpp"
#include "lr_interfaces/msg/car_status.hpp"
#include <iostream>

class CarNode : public rclcpp::Node {
public:
    CarNode(std::string name) : Node(name) {
        client_ = this->create_client<lr_interfaces::srv::Move>("move_car");
        subscription_ = this->create_subscription<lr_interfaces::msg::CarStatus>("car_status", 10, std::bind(&CarNode::status_callback_, this, std::placeholders::_1));
    }

    void send_request(int target) {
        target_height_ = target;     
        is_finished_ = false;

        while (!client_->wait_for_service(std::chrono::seconds(1))) {
            RCLCPP_INFO(this->get_logger(), "等待服务端上线...");
        }
        auto request = std::make_shared<lr_interfaces::srv::Move::Request>();
        request->target_height = target;
        
        // 发送请求
        client_->async_send_request(request);
    }

private:
    rclcpp::Client<lr_interfaces::srv::Move>::SharedPtr client_;
    rclcpp::Subscription<lr_interfaces::msg::CarStatus>::SharedPtr subscription_;
    
    int target_height_ = 0; 
    bool is_finished_ = false;

    // 接收 Topic 回调：加 if-else 判断
    void status_callback_(const lr_interfaces::msg::CarStatus::SharedPtr msg) {
        if (is_finished_) return; // 如果已完成，直接忽略后续消息

        if (msg->current_height >= target_height_) {
            // 达到了目标高度，输出最终结果
            RCLCPP_INFO(this->get_logger(), "最终结果：当前高度 %d 米，共用时 %d 天", msg->current_height, msg->days);
            is_finished_ = true; 
        } else {
            // 没达到，输出当前位置
            RCLCPP_INFO(this->get_logger(), "第 %d 天，当前高度 %d 米", msg->days, msg->current_height);
        }
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CarNode>("car_client");
    
    int target_height;
    std::cout << "请输入目标高度: ";
    std::cin >> target_height;
    
    node->send_request(target_height);
    rclcpp::spin(node);
    
    rclcpp::shutdown();
    return 0;
}