package navi

import (
	"container/heap"
	"encoding/json"
	"fmt"
	"io"
	"math"
	"os"
	"sort"
)

// 定义图结构
type Graph struct {
	Name    string            `json:"name"`
	AdjList map[string][]Edge `json:"adjList"`
	Points  []Point           `json:"points"`
}

type Edge struct {
	Destination int `json:"destination"`
	Weight      int `json:"weight"`
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

// 最小堆实现
type Item struct {
	vertex   int
	priority int
	index    int
}

type PriorityQueue []*Item

func (pq PriorityQueue) Len() int           { return len(pq) }
func (pq PriorityQueue) Less(i, j int) bool { return pq[i].priority < pq[j].priority }
func (pq PriorityQueue) Swap(i, j int)      { pq[i], pq[j] = pq[j], pq[i]; pq[i].index = i; pq[j].index = j }
func (pq *PriorityQueue) Push(x interface{}) {
	n := len(*pq)
	item := x.(*Item)
	item.index = n
	*pq = append(*pq, item)
}
func (pq *PriorityQueue) Pop() interface{} {
	old := *pq
	n := len(old)
	item := old[n-1]
	old[n-1] = nil
	item.index = -1
	*pq = old[0 : n-1]
	return item
}
func dijkstraWithWaypoints(graph *Graph, waypoints []int) ([]int, *Point) {
	fullPath := []int{}
	closestToilet := &Point{Index: -1, Type: "None"}
	closestToiletDistance := math.MaxInt64

	for i := 0; i < len(waypoints)-1; i++ {
		partialPath, tempClosestToilet, tempClosestToiletDistance := dijkstraPath(graph, waypoints[i], waypoints[i+1])
		fullPath = append(fullPath, partialPath[:len(partialPath)-1]...) // Append partial path except the last node to avoid duplication

		// Check and update closest toilet information
		if tempClosestToiletDistance < closestToiletDistance {
			closestToilet = tempClosestToilet
			closestToiletDistance = tempClosestToiletDistance
		}
	}
	fullPath = append(fullPath, waypoints[len(waypoints)-1]) // Append the last target point

	return fullPath, closestToilet
}

func dijkstraPath(graph *Graph, start, target int) ([]int, *Point, int) {
	dist := make(map[int]int)
	prev := make(map[int]int)
	pq := make(PriorityQueue, 0)
	heap.Init(&pq)
	closestToilet := &Point{Index: -1, Type: "None"}
	closestToiletDistance := math.MaxInt64

	for _, p := range graph.Points {
		dist[p.Index] = math.MaxInt64
		prev[p.Index] = -1
		heap.Push(&pq, &Item{vertex: p.Index, priority: math.MaxInt64})
	}

	dist[start] = 0
	heap.Push(&pq, &Item{vertex: start, priority: 0})

	for pq.Len() > 0 {
		u := heap.Pop(&pq).(*Item).vertex
		if u == target {
			break
		}

		for _, edge := range graph.AdjList[fmt.Sprintf("%d", u)] {
			alt := dist[u] + edge.Weight
			if alt < dist[edge.Destination] {
				dist[edge.Destination] = alt
				prev[edge.Destination] = u
				heap.Push(&pq, &Item{vertex: edge.Destination, priority: alt})
			}
		}

		// Update the closest toilet information if this point is a toilet
		if graph.Points[u].Type == "Toilet" && dist[u] < closestToiletDistance {
			closestToilet = &graph.Points[u]
			closestToiletDistance = dist[u]
		}
	}

	// Build path
	path := []int{}
	step := target
	for step != -1 {
		path = append([]int{step}, path...)
		step = prev[step]
	}
	return path, closestToilet, closestToiletDistance
}

// 提取并排序终点相连的景点
func extractAndSortScenic(graph *Graph, target int) []Point {
	scenics := []Point{}

	// 检查与终点直接相连的点
	for _, edge := range graph.AdjList[fmt.Sprintf("%d", target)] {
		point := graph.Points[edge.Destination]
		if point.Type == "Scenic" {
			scenics = append(scenics, point)
		}
	}

	// 根据评分排序
	sort.Slice(scenics, func(i, j int) bool {
		return scenics[i].Rating > scenics[j].Rating // 降序
	})

	return scenics
}

func main() {
	// 读取并解析JSON文件
	jsonFile, err := os.Open("graph.json")
	if err != nil {
		fmt.Println(err)
		return
	}
	defer jsonFile.Close()

	byteValue, _ := io.ReadAll(jsonFile)

	var graph Graph
	json.Unmarshal(byteValue, &graph)

	pointsJSON, _ := json.MarshalIndent(graph.Points, "", "  ")
	os.WriteFile("map.json", pointsJSON, 0644)
	// 计算最短路径和最近的Toilet
	var start *int
	*start = 0
	target := 8
	temp := []int{3}                       // Only one intermediate waypoint
	waypoints := append([]int{0}, temp...) // Start from 0, add intermediate points
	waypoints = append(waypoints, 8)       // Append the final target (8)

	path, closestToilet := dijkstraWithWaypoints(&graph, waypoints)
	fmt.Println("Complete path considering waypoints:", path)
	if closestToilet != nil {
		fmt.Println("Closest Toilet is at point:", closestToilet.Name, "with Index:", closestToilet.Index)
	} else {
		fmt.Println("No Toilet found.")
	}

	pathJSON, _ := json.MarshalIndent(path, "", "  ")
	os.WriteFile("path.json", pathJSON, 0644)
	fmt.Println("Shortest path from", start, "to", target, "is:", path)

	toiletJSON, _ := json.MarshalIndent(closestToilet, "", "  ")
	if closestToilet.Index != -1 {
		fmt.Println("Closest Toilet is at point:", closestToilet.Name, "with Index:", closestToilet.Index)
		os.WriteFile("toilet.json", toiletJSON, 0644)
	} else {
		fmt.Println("No Toilet found.")
		os.WriteFile("toilet.json", []byte("{\"message\": \"No Toilet found.\"}"), 0644)
	}

	// 提取并排序终点相连的景点
	scenics := extractAndSortScenic(&graph, target)
	scenicJSON, _ := json.MarshalIndent(scenics, "", "  ")
	os.WriteFile("scenic.json", scenicJSON, 0644)
	fmt.Println("Scenic points connected to", target, "sorted by rating:")
	for _, scenic := range scenics {
		fmt.Printf("Name: %s, Rating: %.1f\n", scenic.Name, scenic.Rating)
	}
}
