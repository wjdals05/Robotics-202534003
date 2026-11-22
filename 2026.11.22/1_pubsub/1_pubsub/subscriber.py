import rclpy
from rclpy.node import Node
from std_msgs.msg import String



class Subscribe(Node):
    def __init__(self):
        super().__init__('subscriber')
        self.subscription_ = self.create_subscription(String, '/demo_topic', self.callback, 10)
        self.subscription_

    def callback(self, msg2):
        self.get_logger().info(f'robotLocation: "{msg2.data}"')

def main():
    rclpy.init()
    node = Subscribe()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
