--openEditor()
map.setTileSchema("default")

loadMap 'testWilderness'
map.setAmbient(2)
setPalette 'default'
player:teleport(24, 17)


--require 'lua.test'
--player:pushScript(testText)

toggleLightMode()
openEditor()
map.setAmbient(0)
