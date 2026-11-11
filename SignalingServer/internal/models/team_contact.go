package models

import "time"

// TeamContact — общая адресная книга команды: доступ к клиентским станциям,
// который видят все сотрудники под своими личными логинами.
type TeamContact struct {
	ID         uint      `gorm:"primaryKey" json:"id"`
	Name       string    `gorm:"size:128;not null" json:"name"`
	DeviceID   string    `gorm:"size:32;index;not null" json:"device_id"`
	AccessCode string    `gorm:"size:64" json:"access_code"`
	Note       string    `gorm:"size:255" json:"note"`
	CreatedBy  uint      `gorm:"index" json:"created_by"`
	CreatedAt  time.Time `json:"created_at"`
	UpdatedAt  time.Time `json:"updated_at"`
}

func (TeamContact) TableName() string { return "team_contacts" }
