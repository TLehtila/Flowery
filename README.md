# Flowery

Flowery is a continuation of Flower, the COMP3015 coursework one.
Changes from the first coursework are:
- normal map deleted
- spinning flower stayed, plane changed out to a leaf
- flower and leaf appear differently textured, but actually just share a texture atlas
    Texture atlas
![atlas1](https://github.com/TLehtila/Flowery/assets/72321149/903927e0-3ed9-4e0f-b65d-2e6505f19e57)

- edge detection implemented
- gaussian blur implemented, hidden by edges
- particle fountain with mesh objects attempted, does not work with other render passes


In order to see edges, run as is. Edges are done in two colours by using two different edgethreshold uniform variables.  


https://github.com/TLehtila/Flowery/assets/72321149/4443083d-6429-41d9-9517-4c766e297456



In order to see blur, comment out second last line of the main function of basic_uniform.frag shader, and uncomment last line. For the blur to "work" with edges, the setUpFBO2() function of 
scenebasic_uniform.cpp is mashed together from the lab sheets of edge detection and gaussian blur. Render passes 1 and 2 are for edges, 3 and 4 for gaussian blur.
  
![blur1](https://github.com/TLehtila/Flowery/assets/72321149/27a7ed58-a09c-41c7-86b6-b1b20a5f2468)


https://github.com/TLehtila/Flowery/assets/72321149/78ff05b4-de66-4c93-861d-53037ef91234



In order to see particles, comment out all other passes except 0 in the render method of scenebasic_uniform.cpp. Render function of basic_uniform.vert shader is 
split into render() and otherRender(), of which render() does the particle fountain rendering and otherRender() normal rendering. Attempting to use same render function
for both resulted in warped flower and leaves.  
![particles1](https://github.com/TLehtila/Flowery/assets/72321149/5df79d83-087f-42d5-ae45-fd8ccb71c1a4)


https://github.com/TLehtila/Flowery/assets/72321149/4f90ccad-0a9c-4316-a874-9e76e74ecad6

