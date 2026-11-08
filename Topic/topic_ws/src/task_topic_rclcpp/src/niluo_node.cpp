#include"rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class NiluoNode : public rclcpp::Node{
public:
    NiluoNode(std::string name): Node(name){
        RCLCPP_INFO(this->get_logger(),"大家好，我是%s",name.c_str());
        command_publisher_ = this->create_publisher<std_msgs::msg::String>("mingyue",10);
        //话题的名字mingyue
        timer_ =this->create_wall_timer(std::chrono::seconds(5),std::bind(&NiluoNode::timer_callback, this));
        command_subscribe_ = this->create_subscription<std_msgs::msg::String>( "makeboat", 10, std::bind(&NiluoNode::sub_callback, this, std::placeholders::_1));
    }
private:
    //声明话题发布者
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr command_publisher_;
     // 声明订阅者
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr command_subscribe_;
    void sub_callback(const std_msgs::msg::String::SharedPtr msg) {
        //msg 传回数据
        RCLCPP_INFO(this->get_logger(), "收到,%s", msg->data.c_str());
    }
    int times=0;
    void timer_callback(){
        std_msgs::msg::String message;
        message.data = "上课了";
        command_publisher_->publish(message);
        RCLCPP_INFO(this->get_logger(),"第%d节工程设计",times++);
    }
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv){
    rclcpp::init(argc,argv);
    auto node = std::make_shared<NiluoNode>("niluo");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}