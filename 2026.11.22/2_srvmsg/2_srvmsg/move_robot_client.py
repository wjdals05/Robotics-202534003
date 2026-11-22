import sys
import rclpy
from rclpy.node import Node
from custom_interfaces.srv import RobotPose


class Currentpose(Node):
    def __init__(self):
        super().__init__('srv_client')
        self.cli = self.create_client(RobotPose, '/add_two_ints')
        
        while not self.cli.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Waiting for service...')
            
        self.req = RobotPose.Request()

    def send_request(self, x, y,theta,distance):
        
        self.req.x.current_pose = float(x)
        self.req.y.current_pose = float(y)
        self.req.theta.current_pose = float(theta)
        self.req.distance.current_pose = float(distance)
        
        self.future = self.cli.call_async(self.req)
        rclpy.spin_until_future_complete(self, self.future)
        
        return self.future.result()


def main():
    rclpy.init()
    client = Currentpose()
    response = client.send_request(sys.argv[1], sys.argv[2],sys.argv[3],sys.argv[4])
    client.get_logger().info(f'Result pose is : {response.result_pose.x}')
    client.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
