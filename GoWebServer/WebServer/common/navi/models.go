package navi

// 定义图结构
type Graph struct {
	Name    string            `json:"name"`
	AdjList map[string][]Edge `json:"adjList"`
	Points  []Point           `json:"points"`
}

type Edge struct {
	Destination int `json:"destination"`
	Weight      int `json:"weight"`
	Time        int `json:"time"` // 新增时间字段
}

type Point struct {
	Name   string  `json:"name"`
	Index  int     `json:"index"`
	X      int     `json:"x"`
	Y      int     `json:"y"`
	Type   string  `json:"type"`
	Rating float64 `json:"rating"`
	Tag    string  `json:"tag"`
}

type PathResults struct {
	ClosestRes    *Point `json:"closestRes"`
	ClosestToi    *Point `json:"closest_toi"`
	Path          []int  `json:"path"`
	TotalDistance int    `json:"total_distance"`
	TimePath      []int  `json:"time_path"`
	TotalTime     int    `json:"total_time"`
}

// 最小堆实现
type Item struct {
	vertex   int
	priority int
	index    int
}

type PriorityQueue []*Item
