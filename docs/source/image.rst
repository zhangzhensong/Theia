.. highlight:: c++

.. default-domain:: cpp

.. _documentation-image:

=====
Image
=====

Theia provides a basic :class:`Image\<T\>` class to use for SfM and SLAM
applications. The class is very lightweight, and is largely a wrapper for the
`CVD Image class <http://www.edwardrosten.com/cvd/cvd/html/index.html>`_ but
allows for more general use.

.. class:: RGBImage

.. class:: GrayImage

.. class:: Image<T>

  Images can be stored either as RGB or grayscale, denoted as :class:`RGBImage`
  and :class:`GrayImage` respectively. These are simply template specializations
  of the :class:`Image\<T\>` class, so their interfaces will be nearly
  identical. Images can be easily read in from a file upon contruction or with
  the :func:`Read` command.

  .. code-block:: c++

    string img_filename = "..."; // Image you would like to read in.
    RGBImage my_rgb_image(img_filename);

    // Or alternatively:
    RGBImage my_rgb_image;
    my_rgb_image.Read(img_filename);

  Once an image is loaded, pixel values can be accessed with standard ``[]``
  operators. Pixels are referenced in row then column order.

  When accessing image pixels, either a :class:`RGBPixel` or a :class:`Pixel`
  will be returned (for :class:`RBGImage` and :class:`GrayImage`
  respectively). The :class:`Pixel` class is merely a typedef for
  :class:`float`, while the :class:`RGBPixel` contains members
  :member:`red`, :member:`green`, and :member:`blue` (all type :class:`float`).

  .. code-block:: c++
  
    // Load RGB and Grayscale images.
    RGBImage my_rgb_img("test_img.jpg");    
    GrayImage my_gray_img("test_img.jpg");
    
    // Get the middle pixel location.
    int middle_row = my_image.Rows()/2;
    int middle_col = my_image.Cols()/2;

    // Grab the middle pixel.
    RGBPixel middle_rgb_pixel = my_rgb_img[middle_row][middle_col];
    Pixel middle_gray_pixel = my_gray_img[middle_row][middle_col];

    // Output the RGB Pixel value.
    std::cout << "red = " << middle_rgb_pixel.red 
              << " green = " << middle_rgb_pixel.green
              << " blue = " << middle_rgb_pixel.blue << std::endl;
	      
    // Output the grayscale pixel value.
    std::cout << "gray = " << middle_gray_pixel << std::endl;

  You can also extra patches from images using the :func:`GetSubImage` function.

  .. function:: SubImage\<T\> GetSubImage(int row, int col, int num_rows, int num_cols)

    Extracts a sub-image at the given row, col of the size (num_rows,
    num_cols). The row, col given should denote the top-left corner of the
    subwindow to extract. The extracted sub-image will be a reference to the
    patch in the original image, not a copy of it. However, the :class:`Image`
    class that the patch was extracted from will still own the image so care
    must be taken when deleting images. :func:`GetSubImage` will return a
    :class:`SubImage\<T\>` object of the same type that it is extracting from.

  .. code-block:: c++

    RGBImage my_img("...");

    // Get the middle pixel location.
    int middle_row = my_image.Rows()/2;
    int middle_col = my_image.Cols()/2;

    // Extract a 15x15 subimage centered at the middle pixel.    
    int patch_size = 15;
    RGBSubImage middle_sub_img = my_img.SubImage(middle_row - patch_size/2,
                                                 middle_col - patch_size/2,
		 			         patch_size,
		 			         patch_size);

.. class:: SubImage<T>

.. class:: RGBSubImage

.. class:: GraySubImage
  