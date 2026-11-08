#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class StudentNode : public rclcpp::Node
{
public:
    StudentNode(std::string name) : Node(name)
    {
        RCLCPP_INFO(this->get_logger(), "大家好，我是%s.", name.c_str());
        command_subscribe_ = this->create_subscription<std_msgs::msg::String>("mingyue", 10, std::bind(&StudentNode::command_callback, this, std::placeholders::_1));
        command_publisher_ = this->create_publisher<std_msgs::msg::String>("makeboat", 10);
    }
private:
    int weight=0;
     // 声明订阅者
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr command_subscribe_;
    //声明话题发布者
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr command_publisher_;
    void command_callback(const std_msgs::msg::String::SharedPtr msg)
    {


        RCLCPP_INFO(this->get_logger(), "%s,我要多做1kg船",msg->data.c_str());
        std_msgs::msg::String boat_msg;
        weight++; 
        boat_msg.data = "船重 " + std::to_string(weight) + " kg";
        command_publisher_->publish(boat_msg);
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<StudentNode>("waya");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
