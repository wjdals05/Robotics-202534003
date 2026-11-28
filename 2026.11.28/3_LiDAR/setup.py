from setuptools import find_packages, setup

package_name = '3_LiDAR'

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
            'simple_oa = 3_LiDAR.simple_oa:main',
            'simple_yaw_oa = 3_LiDAR.simple_yaw_oa:main',

        ],
    },
)
