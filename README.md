# Seam Carving

Seam carving is a method for dynamic image resizing. In this implementation, I use C++ and OpenCV.

Seam carving works by finding low energy seams (single pixel paths from top-bottom or left-right) in an image, and then removing them. Basically, energy seams are ones with minimal total variation between all pixels in the seam. The idea is that by proritizing the removal of low energy seams, the objects of focus in an image will not be visually disrupted when resizing.

### Getting Energy Image

To get the energy image, I used the Scharr operator to generate a gradient magnitude drawing of the original image. Here is the original:
![Original](https://github.com/lucasleschynski/seam-carving/blob/main/images/readme/original.png?raw=true)

And here is the original converted into an energy image:

![Energy](https://github.com/lucasleschynski/seam-carving/blob/main/images/readme/energy.png?raw=true)

### Finding Optimal Seams

After getting the energy image, the optimal seams are computed. In order to do this, the total energy for all seams must be found.

To do this efficiently, the image is scanned from top to bottom, generating the total energy for each seam. Dynamic programming is used here to make the computation of the seams efficient.

After finding the total energy for each seam, we have something like this:
![Color](https://github.com/lucasleschynski/seam-carving/blob/main/images/readme/color.png?raw=true)

This shows the energy of each seam from top to bottom at each pixel in its path (red = high energy, blue = low energy).

The optimal seam is the one that has a bottom pixel with the lowest total energy. Once we know which pixel this is, we can reconstruct the optimal seam by backtracking through the image and recording the location of each pixel in the seam.

### Removing seams

Once we have an optimal seam, we simply remove it. If we want to remove more than one seam (as we often do), we repeat the process(getting energy image, ... ,removing the seam).

For example, if we choose to remove 350 pixels from the width of the original image above, we get:

![Color](https://github.com/lucasleschynski/seam-carving/blob/main/images/readme/resized.png?raw=true)
