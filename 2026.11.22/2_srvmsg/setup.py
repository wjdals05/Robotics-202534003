from setuptools import find_packages, setup

package_name = '2_srvmsg'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='edu45',
    maintainer_email='wjdals05@gachon.ac.kr',
    description='TODO: Package description',
    license='TODO: License declaration',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'add_ints_client = 2_srvmsg.add_ints_client:main',
            'add_ints_server = 2_srvmsg.add_ints_server:main',
            'move_robot_client = 2_srvmsg.move_robot_client:main',
            'move_robot_server = 2_srvmsg.move_robot_server:main',
        ],
    },
)
