# Seam Carving

Seam carving is a method for dynamic image resizing. Here, C++ and OpenCV are used.

Seam carving works by finding "low energy" seams in an image, and then removing them. Put simply, low energy seams are ones with minimal total variation between member pixels. The idea is that by proritizing the removal of low energy seams, images can be resized without visually disrupting the objects of focus.

### Getting Energy Image

To get the energy image, the Scharr operator is used. Here is the original:

![Original](https://github.com/lucasleschynski/seam-carving/blob/main/images/readme/original.png?raw=true)

<!-- <img src="https://github.com/lucasleschynski/seam-carving/blob/main/images/readme/original.png?raw=true" height="280" /> -->

And here is the transformed energy image:

![Energy](https://github.com/lucasleschynski/seam-carving/blob/main/images/readme/energy.png?raw=true)

### Finding Optimal Seams

After getting the energy image, the optimal seams are computed.

This can be done efficiently with dynamic programming -- the image is scanned from top to bottom, finding the total energy for each seam. In this process, previously calculated seam "weights" are built upon to eliminate the need for manually computing each seam individually. 

After finding the total energy for each seam, we have something like this:
![Color](https://github.com/lucasleschynski/seam-carving/blob/main/images/readme/color.png?raw=true)

This shows the energy of each seam from top to bottom at each pixel in its path (red = high energy, blue = low energy).

The optimal seam is the one that has a bottom pixel with the lowest total energy. Once we know which pixel this is,  the lowest energy seam can be reconstructed by backtracking through the image and recording the location of each pixel in the seam.

### Removing seams

Once we have an optimal seam, we simply remove it from the original image. If we want to remove more than one seam (as we often do), we repeat the process(getting energy image, computing low energy seam, etc.).

For example, if we choose to remove 350 pixels from the width of the original image above, we get:

![Color](https://github.com/lucasleschynski/seam-carving/blob/main/images/readme/resized.png?raw=true)

<!-- <img src="https://github.com/lucasleschynski/seam-carving/blob/main/images/readme/resized.png?raw=true" height="275" /> -->
