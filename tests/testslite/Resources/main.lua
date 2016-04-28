
print("hello")

local function main()

    local function printf(fmt, ...)
        print(string.format(fmt, ...))
    end

    local function dump(o, label)
        printf("---- %s ----", label or "<var>")
        for k, v in pairs(o) do
            printf("  %s = %s", tostring(k), tostring(v))
        end
    end

    print("Hello, Lua Main")
    printf("g_layer = %s", tostring(g_layer))

    local HelloWorldLayer = {}

    local centerx = g_viewsize_width / 2
    local centery = g_viewsize_height / 2

    function HelloWorldLayer:ctor(parent)
        self.parent = parent
        self.starsLayer = cc.Node.create()
        self.parent:addChild(self.starsLayer)

        local layerColor = cc.LayerColor.create(cc.Color4B.new(200, 200, 200, 200), 200, 40)
        layerColor:setPosition(centerx - 100, g_viewsize_height - 40)
        self.parent:addChild(layerColor)

        self.starsLabel = cc.Label.createWithSystemFont("0 stars", "sans", 24)
        self.starsLabel:setColor(cc.Color3B.new(0, 0, 0))
        self.starsLabel:setPosition(100, 20)
        layerColor:addChild(self.starsLabel)

        self.offsetCount = 60
        self.offsets = {}
        for i = 0, 59 do
            self.offsets[i] = {
                x = math.sin(i * 6 * math.pi / 180) * 4,
                y = math.cos(i * 6 * math.pi / 180) * 4,
            }
        end

        self.maxStars = 20000
        self.starsCountOffset = 100
        self.stars = {}
        self.stepsCount = 300
        self.steps = self.stepsCount
        self.starsLayer:schedule(function(dt)
            self:update(dt)
        end, 1.0 / 60, "update")

        self:addStars(7500)
    end

    function HelloWorldLayer:addStars(count)
        for i = 1, count do
            local star = {}

            star.sprite = cc.Sprite.create("star.png")
            star.node = lgetNode(star.sprite)
            star.pos = {
                x = math.random() * g_viewsize_width,
                y = math.random() * g_viewsize_height,
                i = math.floor(math.random() * self.offsetCount),
                o = math.floor(math.random() * 256),
                oi = 1
            }
            local offset = self.offsets[star.pos.i]
            star.sprite:setPosition(star.pos.x + offset.x, star.pos.y + offset.y)
            star.sprite:setOpacity(star.pos.o)
            self.starsLayer:addChild(star.sprite)

            self.stars[#self.stars + 1] = star
        end

        if #self.stars >= self.maxStars then
            self.starsCountOffset = -self.starsCountOffset
        end
    end

    function HelloWorldLayer:removeStars(count)
        while count > 0 and #self.stars > 0 do
            local star = table.remove(self.stars)
            self.starsLayer:removeChild(star.sprite)
            count = count - 1
        end

        if #self.stars <= 0 then
            self.starsCountOffset = -self.starsCountOffset
        end
    end

    function HelloWorldLayer:update(dt)
        self.steps = self.steps + 1
        if self.steps >= self.stepsCount then
            if self.starsCountOffset > 0 then
                self:addStars(self.starsCountOffset)
            else
                self:removeStars(-self.starsCountOffset)
            end
            self.steps = 0
            self.starsLabel:setString(tostring(#self.stars) .. " stars")
        end

        local updateStar = self.updateStar
        for i = 1, #self.stars do
            updateStar(self, self.stars[i])
        end
    end

    local lsetPosition = lsetPosition
    local lsetOpacity = lsetOpacity

    local pos, offset, offsetCount, node

    function HelloWorldLayer:updateStar(star)
        pos = star.pos
        offset = self.offsets[pos.i]
        offsetCount = self.offsetCount

        pos.i = pos.i + 1
        pos.i = pos.i % offsetCount
        pos.o = pos.o + pos.oi
        if pos.o > 255 then
            pos.o = 255
            pos.oi = -pos.oi
        elseif pos.o < 0 then
            pos.o = 0
            pos.oi = -pos.oi
        end

        node = star.node

        -- lsetPosition(node, pos.x + offset.x, pos.y + offset.y)
        -- lsetOpacity(node, pos.o)

        star.sprite:setPosition(pos.x + offset.x, pos.y + offset.y)
        star.sprite:setOpacity(pos.o)
    end

    collectgarbage("stop")

    local layer = {}
    setmetatable(layer, {__index = HelloWorldLayer})
    layer:ctor(g_layer)
end

xpcall(main, function(err)
    print("")
    print("LUA ERROR")
    print("")
    print(err)
    print("")
    print(debug.traceback())
end)
