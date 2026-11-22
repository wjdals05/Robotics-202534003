import rclpy
from rclpy.node import Node
from custom_interfaces.srv import AddTwoInts


class AddTwoIntsServer(Node):
    def __init__(self):
        super().__init__('srv_server')

        self.srv = self.create_service(AddTwoInts, '/add_two_ints', self.srv_callback)
        self.get_logger().info('Srv Server Ready.')

    def srv_callback(self, request, response):
        
        
        self.get_logger().info(f'Incoming request\na: {request.a.my_num}, b: {request.b.my_num}')
        self.get_logger().info(f'Sending back response: [{response.sum.my_num}]')
        
        return response


def main():
    rclpy.init()
    node = AddTwoIntsServer()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
