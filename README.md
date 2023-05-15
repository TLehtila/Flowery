# Flowery

Flowery is a continuation of Flower, the COMP3015 coursework one.
Changes from the first coursework are:
- normal map deleted
- spinning flower stayed, plane changed out to a leaf
- flower and leaf appear differently textured, but actually just share a texture atlas
- edge detection implemented
- gaussian blur implemented, hidden by edges
- particle fountain with mesh objects attempted, does not work with other render passes


In order to see edges, run as is. Edges are done in two colours by using two different edgethreshold uniform variables.  


In order to see blur, comment out second last line of basic_uniform.frag shader, and uncomment last line. For the blur to "work" with edges, the setUpFBO2() function of 
scenebasic_uniform.cpp is mashed together from the lab sheets of edge detection and gaussian blur. Render passes 1 and 2 are for edges, 3 and 4 for gaussian blur.
  

In order to see particles, comment out all other passes except 0 in the render method of scenebasic_uniform.cpp. Render function of basic_uniform.vert shader is 
split into render() and otherRender(), of which render() does the particle fountain rendering and otherRender() normal rendering. Attempting to use same render function
for both resulted in warped flower and leaves.  
