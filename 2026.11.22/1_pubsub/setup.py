from setuptools import find_packages, setup

package_name = '1_pubsub'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name + '/launch',            # 수정
            ['launch/my_lacation.launch.py'])            # 수정
        
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
            'talker = 1_pubsub.talker:main',   # 수정
            'listener = 1_pubsub.listener:main',
            'publisher = 1_pubsub.pubilsher:main',
            'processor = 1_pubsub.processor:main',
            'subscriber = 1_pubsub.subscriber:main'
        ],
    },
)
