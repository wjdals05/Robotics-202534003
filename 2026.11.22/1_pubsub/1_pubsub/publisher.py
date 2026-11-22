import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import time



class DirectionPublish(Node):
    def __init__(self):
        super().__init__('publisher')
        self.publisher_ = self.create_publisher(String, '/demo_topic', 10)
        self.start_time = time.time()
        self.timer = self.create_timer(0.1, self.publish_message)

    def publish_message(self):
        direction = input('input---[w,a,s,d]:')
        msg = String()
        msg.data = f"This is: {direction}"
        self.publisher_.publish(msg)
        self.get_logger().info(f'direction: "{msg.data}"')
    

    




def main():
    rclpy.init()
    node = DirectionPublish()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
