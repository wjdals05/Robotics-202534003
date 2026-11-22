from launch import LaunchDescription
from launch.actions import ExecuteProcess, RegisterEventHandler
from launch.event_handlers import OnProcessStart


def generate_launch_description(): #Don't fix the name of the function

    publisher = ExecuteProcess(
        cmd=[
            'gnome-terminal', '--', 'bash', '-c',
            'source ~/robot_navigation/colcon_ws/install/setup.bash; '
            'ros2 run 1_pubsub publisher; '
            'exec bash'
        ],
        output='screen'
    )

    processor = ExecuteProcess(
        cmd=[
            'gnome-terminal', '--', 'bash', '-c',
            'source ~/robot_navigation/colcon_ws/install/setup.bash; '
            'ros2 run 1_pubsub processor; '
            'exec bash'
        ],
        output='screen'
    )

    subscriber = ExecuteProcess(
        cmd=[
            'gnome-terminal', '--', 'bash', '-c',
            'source ~/robot_navigation/colcon_ws/install/setup.bash; '
            'ros2 run 1_pubsub subscriber; '
            'exec bash'
        ],
        output='screen'
    )

    

    return LaunchDescription([
        publisher,

        RegisterEventHandler(
            OnProcessStart(
                target_action=publisher,
                on_start=[processor]
            )
        ),

        RegisterEventHandler(
            OnProcessStart(
                target_action=processor,
                on_start=[subscriber]
            )
        ),
    ])