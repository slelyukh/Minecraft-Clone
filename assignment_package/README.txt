Certain aspects of the game are disabed to imporve performance. This readme will tell you how to
enable these parts.

Rivers: Take 30 seconds to generate at the start but don't reduce frame rate once generated.
Enable by making sure m_terrain.createRiver(ivec2(0,0), 45.f) is uncommented in initialize gl in mygl
(~line 95)

Caves: Reduces frames by a lot since all underground block faces are loaded.
Enable by make sure these two lines are called in blocktype worker:
    BlockType b = HeightMap::getDepth(x+xl,yl,z+zl);
    m_chunk->setBlockAt(xl,yl,zl, b);
(~line 53)
