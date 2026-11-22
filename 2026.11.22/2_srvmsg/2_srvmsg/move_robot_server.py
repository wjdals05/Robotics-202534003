import rclpy
from rclpy.node import Node
from custom_interfaces.srv import MoveRobot


class AddTwoIntsServer(Node):
    def __init__(self):
        super().__init__('srv_server')

        self.srv = self.create_service(MoveRobot, '/add_two_ints', self.srv_callback)
        self.get_logger().info('Srv Server Ready.')

        
    def srv_callback(self, request, response):
        x= request.x.current_pose
        y=request.y.current_pose
        theta = request.theta.current_pose
        distance= request.theta.distance
    
        
        self.get_logger().info(f'Incoming request\na: {request.x.current_pose }, b: {request.y.current_pose}')
        self.get_logger().info(f'Sending back response: [{response.result_pose.pose}]')
        self.get_logger().info(f'Sending back response: [{response.bool.success}')
        return response


def main():
    rclpy.init()
    node = AddTwoIntsServer()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
