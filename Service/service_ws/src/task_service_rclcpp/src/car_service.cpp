#include "rclcpp/rclcpp.hpp"
#include "lr_interfaces/srv/move.hpp"
#include "lr_interfaces/msg/car_status.hpp"
#include <chrono>

class CarService : public rclcpp::Node {
public:
    CarService(std::string name) : Node(name) {
        server_ = this->create_service<lr_interfaces::srv::Move>(
            "move_car", 
            std::bind(&CarService::handle_move, this, std::placeholders::_1, std::placeholders::_2));
        publisher_ = this->create_publisher<lr_interfaces::msg::CarStatus>("car_status", 10);
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(500), 
            std::bind(&CarService::timer_callback, this));
    }

private:
    rclcpp::Service<lr_interfaces::srv::Move>::SharedPtr server_;
    rclcpp::Publisher<lr_interfaces::msg::CarStatus>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

    int current_height = 0;
    int target_height = 0;
    int days = 0;
    bool is_moving = false;

    void handle_move(const std::shared_ptr<lr_interfaces::srv::Move::Request> request,
                     std::shared_ptr<lr_interfaces::srv::Move::Response> response) {
        RCLCPP_INFO(this->get_logger(), "收到目标高度: %d", request->target_height);
        
        current_height = 0;
        days = 0;
        target_height = request->target_height;
        is_moving = true; 
        
        response->current_height = 0;
        response->days = 0;
    }

    void timer_callback() {
        if (is_moving) {
            if (current_height < target_height) {
                current_height += 3;
                days++;
                RCLCPP_INFO(this->get_logger(), "移动一次");
                auto msg = lr_interfaces::msg::CarStatus();
                msg.current_height = current_height;
                msg.days = days;
                publisher_->publish(msg);
            } else {
                is_moving = false; 
            }
        }
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CarService>("car_service");
    rclcpp::spin(node); 
    rclcpp::shutdown();
    return 0;
}