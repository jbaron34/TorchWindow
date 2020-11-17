from setuptools import setup
from setuptools.dist import Distribution


class BinaryDistribution(Distribution):
    """Distribution which always forces a binary package with platform name"""
    def has_ext_modules(foo):
        return True


setup(
    name='TorchWindow',
    version='0.1.0',
    author='jbaron34',
    author_email='jbaron34@gmail.com',
    packages=['torchwindow'],
    package_data={
        'torchwindow': [
            'twinterface.dll',
            'torchwindow.exe',
            'frag.spv',
            'vert.spv',
            'SDL2.dll'
        ]
    },
    platforms=['x64-Windows'],
    url='https://github.com/jbaron34/TorchWindow',
    license='MIT',
    description='Displays PyTorch image tensors in a window on screen',
    long_description_content_type='text/markdown',
    long_description=open('README.md').read(),
    install_requires=[
        "torch >= 0.4"
    ],
    classifiers=[
        "License :: OSI Approved :: MIT License",
        "Development Status :: 3 - Alpha",
        "Environment :: GPU",
        "Environment :: GPU :: NVIDIA CUDA",
        "Environment :: Win32 (MS Windows)",
        "Intended Audience :: Developers",
        "Operating System :: Microsoft :: Windows",
        "Programming Language :: C",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3 :: Only",
        "Topic :: Multimedia :: Graphics :: Viewers"
    ],
    python_requires='>=3',
    distclass=BinaryDistribution
)