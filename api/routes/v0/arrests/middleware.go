package arrests

import (
    "github.com/gin-gonic/gin"
)

func MIDDLEWARE(c *gin.Context) {
    c.Next()
}