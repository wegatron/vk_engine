## download gltf data
https://sketchfab.com/3d-models/girl-warrior-motorcycle-cyber-by-oscar-creativo-c27030b41da04de29b0ba78c486a9b31#download

## For
The descriptor set number 0 will be used for engine-global resources, and bound once per frame. The descriptor set number 1 will be used for per-pass resources, and bound once per pass. The descriptor set number 2 will be used for material resources, and the number 3 will be used for per-object resources. This way, the inner render loops will only be binding descriptor sets 2 and 3, and performance will be high.