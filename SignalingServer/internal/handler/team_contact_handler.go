package handler

import (
	"net/http"
	"strconv"
	"strings"

	"quickdesk/signaling/internal/middleware"
	"quickdesk/signaling/internal/models"

	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

// TeamContactHandler — общая адресная книга: клиентские станции с кодами доступа,
// доступные всем сотрудникам под личными логинами. Любой авторизованный
// сотрудник может добавить запись; менять и удалять может автор или админ.
type TeamContactHandler struct {
	db *gorm.DB
}

func NewTeamContactHandler(db *gorm.DB) *TeamContactHandler {
	return &TeamContactHandler{db: db}
}

type teamContactReq struct {
	Name       string `json:"name"`
	DeviceID   string `json:"device_id" binding:"required"`
	AccessCode string `json:"access_code"`
	Note       string `json:"note"`
}

func (h *TeamContactHandler) List(c *gin.Context) {
	var items []models.TeamContact
	if err := h.db.WithContext(c.Request.Context()).Order("name asc, id asc").Find(&items).Error; err != nil {
		ProblemInternal(c, err.Error())
		return
	}
	c.JSON(http.StatusOK, gin.H{"items": items})
}

func (h *TeamContactHandler) Create(c *gin.Context) {
	uid := middleware.MustUserID(c)
	var req teamContactReq
	if err := c.ShouldBindJSON(&req); err != nil {
		ProblemBadRequest(c, ProblemCodeInvalidRequest, err.Error())
		return
	}
	deviceID := strings.TrimSpace(req.DeviceID)
	if deviceID == "" {
		ProblemBadRequest(c, ProblemCodeInvalidRequest, "device_id is required")
		return
	}
	item := models.TeamContact{
		Name:       strings.TrimSpace(req.Name),
		DeviceID:   deviceID,
		AccessCode: strings.TrimSpace(req.AccessCode),
		Note:       strings.TrimSpace(req.Note),
		CreatedBy:  uid,
	}
	if item.Name == "" {
		item.Name = "Станция " + deviceID
	}
	if err := h.db.WithContext(c.Request.Context()).Create(&item).Error; err != nil {
		ProblemInternal(c, err.Error())
		return
	}
	c.JSON(http.StatusCreated, item)
}

func (h *TeamContactHandler) Update(c *gin.Context) {
	uid := middleware.MustUserID(c)
	id, err := strconv.Atoi(c.Param("id"))
	if err != nil {
		ProblemBadRequest(c, ProblemCodeInvalidRequest, "bad id")
		return
	}
	var item models.TeamContact
	if err := h.db.WithContext(c.Request.Context()).First(&item, id).Error; err != nil {
		ProblemNotFound(c, "contact not found")
		return
	}
	if item.CreatedBy != uid {
		ProblemForbidden(c, "only the author can change this entry")
		return
	}
	var req teamContactReq
	if err := c.ShouldBindJSON(&req); err != nil {
		ProblemBadRequest(c, ProblemCodeInvalidRequest, err.Error())
		return
	}
	item.Name = strings.TrimSpace(req.Name)
	item.DeviceID = strings.TrimSpace(req.DeviceID)
	item.AccessCode = strings.TrimSpace(req.AccessCode)
	item.Note = strings.TrimSpace(req.Note)
	if err := h.db.WithContext(c.Request.Context()).Save(&item).Error; err != nil {
		ProblemInternal(c, err.Error())
		return
	}
	c.JSON(http.StatusOK, item)
}

func (h *TeamContactHandler) Delete(c *gin.Context) {
	uid := middleware.MustUserID(c)
	id, err := strconv.Atoi(c.Param("id"))
	if err != nil {
		ProblemBadRequest(c, ProblemCodeInvalidRequest, "bad id")
		return
	}
	var item models.TeamContact
	if err := h.db.WithContext(c.Request.Context()).First(&item, id).Error; err != nil {
		ProblemNotFound(c, "contact not found")
		return
	}
	if item.CreatedBy != uid {
		ProblemForbidden(c, "only the author can delete this entry")
		return
	}
	if err := h.db.WithContext(c.Request.Context()).Delete(&item).Error; err != nil {
		ProblemInternal(c, err.Error())
		return
	}
	c.JSON(http.StatusOK, gin.H{"status": "ok"})
}
