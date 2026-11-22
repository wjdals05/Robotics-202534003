import rclpy
from rclpy.node import Node
from std_msgs.msg import String



class Processor(Node):
    mylocation=[0,0]


    
    def __init__(self):
        super().__init__('processor')
        self.subscription_ = self.create_subscription(String, '/demo_topic', self.callback, 10)
        self.subscription_
        self.mylocation=[0,0]

    def callback(self, msg):
        if msg=='w': mylocation=mylocation + [0,1]
        if msg=='s': mylocation=mylocation + [0,-1]
        if msg=='a': mylocation=mylocation + [-1,0]
        if msg=='d': mylocation=mylocation + [1,0]         
        self.get_logger().info(f'Received: "{msg.data}"')



    def publish_message(self):
        self.mylocation
        msg2= String()
        msg2.data = f"move: {self.mylocation}"
        self.publisher_.publish(msg2)
        self.get_logger().info(f'Publishing: "{msg2.data}"')
    

def main():
    rclpy.init()
    node = Processor()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
