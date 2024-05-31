package recommend

import (
	models "gowebserver/WebServer/model"
	"math"
	"sort"
)

// 协同推荐类别,按照类别过滤且按照热度排序
// 可以返回tags字符串,并且只要得到一位用户的东西就可以了,这里返回前三个tag

type RtDist struct {
	UserId string
	Dist   float64
}

func binarySearch(dm []RtDist, t RtDist, n int) int {
	var l int = 0
	var r = n
	for l < r {
		mid := (l + r) >> 1
		if dm[mid].Dist > t.Dist {
			r = mid - 1
		} else if dm[mid].Dist < t.Dist {
			l = mid + 1
		} else {
			return mid
		}
	}
	return l
}

func getDist(rt1, rt2 models.Rating) RtDist {
	var dist float64 = 0.0

	dist += (rt1.Art - rt2.Art) * (rt1.Art - rt2.Art)
	dist += (rt1.Culture - rt2.Culture) * (rt1.Culture - rt2.Culture)
	dist += (rt1.Education - rt2.Education) * (rt1.Education - rt2.Education)
	dist += (rt1.Exploration - rt2.Exploration) * (rt1.Exploration - rt2.Exploration)
	dist += (rt1.History - rt2.History) * (rt1.History - rt2.History)
	dist += (rt1.Nature - rt2.Nature) * (rt1.Nature - rt2.Nature)
	dist += (rt1.Science - rt2.Science) * (rt1.Science - rt2.Science)

	return RtDist{
		UserId: rt2.UserId,
		Dist:   math.Sqrt(dist),
	}
}

// insert sort + divide & conquer
func getNeighbors(rt models.Rating, userRt []models.Rating, userNum int) []RtDist {
	var len int = int(math.Min(float64(userNum/2), 50))
	var n int = 0
	var distMatrix []RtDist
	for _, ur := range userRt {
		r := getDist(rt, ur)
		if n == 0 || r.Dist >= distMatrix[n-1].Dist && len > n {
			distMatrix = append(distMatrix, r)
			n++
		} else if r.Dist < distMatrix[n-1].Dist && len > n {
			distMatrix = append(distMatrix, r)
			idx := binarySearch(distMatrix, r, n-1)
			for i := n - 1; i >= idx; i-- {
				distMatrix[i+1] = distMatrix[i]
			}
			distMatrix[idx] = r
			n++
		} else if r.Dist < distMatrix[n-1].Dist && len <= n {
			distMatrix[n-1] = r
			idx := binarySearch(distMatrix, r, n-1)
			for i := n - 1; i >= idx; i-- {
				distMatrix[i+1] = distMatrix[i]
			}
			distMatrix[idx] = r
		}
	}
	return distMatrix
}

func getUserRating(userId string, userRt []models.Rating) models.Rating {
	for _, rt := range userRt {
		if rt.UserId == userId {
			return rt
		}
	}
	return models.Rating{}
}

func GenerateRcmd(rt models.Rating, userRt []models.Rating, userNum int) (tags []string) {
	neighbors := getNeighbors(rt, userRt, userNum)

	tagScores := make(map[string]float64)
	for _, neighbor := range neighbors {
		weight := 1.0
		neighborRt := getUserRating(neighbor.UserId, userRt)
		tagScores["Art"] += weight * neighborRt.Art
		tagScores["Culture"] += weight * neighborRt.Culture
		tagScores["Education"] += weight * neighborRt.Education
		tagScores["Exploration"] += weight * neighborRt.Exploration
		tagScores["History"] += weight * neighborRt.History
		tagScores["Nature"] += weight * neighborRt.Nature
		tagScores["Science"] += weight * neighborRt.Science
	}

	type kv struct {
		Key   string
		Value float64
	}
	var sortedTags []kv
	for k, v := range tagScores {
		sortedTags = append(sortedTags, kv{k, v})
	}
	sort.Slice(sortedTags, func(i, j int) bool {
		return sortedTags[i].Value > sortedTags[j].Value
	})

	maxTags := 3
	for i := 0; i < maxTags && i < len(sortedTags); i++ {
		tags = append(tags, sortedTags[i].Key)
	}

	return tags
}