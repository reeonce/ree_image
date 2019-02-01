# ree_image

## how to use?

```cpp
auto source = ree::io::Source::SourceByPath("./test/lena.jpg");
ree::image::Image *img;
int ret = ree::image::FileFormat::Parse(source.get(), &img);
if (ret != ree::image::ErrorCode::OK) {
    // error handling
} else {
    // display the image
}
```