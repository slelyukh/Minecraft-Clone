Player physics implementation
-Gravity
	I check the block underneath by using the playerpos (which is at the feet of the player) and decreased the y value
	by a small amount. Then I used terrain.getblock to check if the block was empty or not. If so, I made my acceleration
	vector in the y direction -1
-Player Collision
	I implemented this as suggested in class by using the grid march function given in class to send a ray originating from
	each of the 12 corners of the player. If the ray hit a block, then I would adjust the movement amount to move to just up
	to the block.
-Block Removing
	I used the grid march method again here but originating from the camera position and using the forward vector multiplied
	by 3 so that I would grid march 3 units away
-Block Placing
	I used the grid march method here again but with some more math to figure out which face to place the block on. Specifically,
	I got the point on the block we are placing on by going along the forward vector by the returned distance amount. Then,
	I subtracted the block origin from the point and checked if any of the x,y,z values here are 0 or -1/+1. This gives enough
	information to figure out which face.