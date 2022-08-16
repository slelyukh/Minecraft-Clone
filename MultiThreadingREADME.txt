To implement multithreading for faster terrain generation I created two ThreadWorkers:
BlockTypeWorkers to concurrently fill up the block type vectors of new chunks with the
correct blocks based on the coordinates of the chunk and the random terrain
and VBOWorkers to concurrently create Vertex Buffer Objects(VBOs) for each newly
generated chunk to render it.

To store chunks that have block data and chunks that have VBO data I made a vectors of
each and used QMutexes to lock and unlock these vectors when threads read/write to them.
At every tick in the game, first the game checks whether the player has moved enough to warrant
the creation or deletion of certain chunks' block data or VBO data. Only a 48x48 zone of chunks
around the player are rendered at a time if the player is still. During each tick, every chunk that needs block
data is taken by an individual BlockTypeWorker and every chunk that has block data but needs VBO data is taken 
by a VBOWorker to create the VBO. Also, chunks with processed VBOs are rendered.