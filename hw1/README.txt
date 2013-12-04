README

Ricardo Zeballos CS184-ea 
TriMan Nguyen	 cs184-ac

HOMEWORK 3 Link to web site {http://inst.cs.berkeley.edu/~CS184-ea/Homework_3}

In homework 3 we created a aparment scene. 
The requirements that are fulfilled will be shown as numbers 1) being the first requirement and 20) being the last.

1) As can be seen the aparment contains alot of small possesions as well as a small table, statues and two lamps.
The scene also contains a study desk and a desk with drawers.  
2,11) The small table was created by hand using vertex locations and moving the legs around using different view matrixes. 
Once everything is together the whole table in then moved into location using a single matrix transformation.
3)We parsed a Drawer Model which comes out a bit funky do to the direction in which the vertexies are read.
4)We parsed a Desk Model which can be seen on the right.
5,13) All the objects are scaled rotated and placed by hand if you take a look our extensive display method it does all such placements manually.
And also the normal vectors are also place manually of all the objects drawn. On the parsed objects the normals are intergated in the loop that sets
up all the faces of said object.
6) The floor is textured using a bmp image and a SOIL loader for said image, in the fragement shader the lights are taken into consideration to give
our textured object the best look possible.
7) The second object to be textured is the spining cube located between the statues. This was also a bmp texture.
8) If you take a look straight ahead you will see the two sphere statues on of which seems very dull in comparison to the other. We did this by playing around 
with the fragshader values that are passed in such as ambient, specular, ect.
9) If you look straight ahead you can see the point light reflection off the walls. This light was placed in the middle of the scene to give maximun lighting.
10) The directional light is harder to spot since we are in doors but it is definately there in our light declaration in main it can be seen that this light is declared with
a w value of 0. In the fragment shader this light is treated as a directional light correctly.
12) The left lamp will swith to wire frame rendering given you press capital W.
14) The use of prespective projection can be easily spotted as things seems to be heading torwars a vanishing point. The code to do this is in the reshape method 
as can be seen we use the correct prespective functions. The double buffering as well as the hidden surface elimination can be seen in our main method in the glutInitDisplayMode call.
All this again can be seen by the quality of our rendering in our scene.
15)To turn textures on and off use the capital I key.
To turn lights on and off use the capital L key.
16,17)If you take a look at the right wall it has a brick texture as well and a bump map effect. It is difficult to see at first do to the face no shading was implemented however 
if you compare the light reflection to that of the oposite wall you can see that light if being reflected differently at different portions of the wall. This is done do to 
the normals that were pulled from the bumpMap texture.
18,19) In our scene the user is given a first person shooter control scheme the wasd controls for movement and the arrow keys for rotation. For more details look at termial print out.
In addition to those controls we offer z to move down and x to move up. And given the position of the camera the z and x can also be used for tilting the the z->left and x->right. 
Also if you turn on mouse controls you will be able to use the mouse and rotate using the mouse. By clicking on the screen in the position you want to rotate. 
20) To turn on animations press capital P. Both the cube in the center and the octagons will become animated.

