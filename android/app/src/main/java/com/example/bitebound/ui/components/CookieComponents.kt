package com.example.bitebound.ui.components

import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.example.bitebound.data.GameConfigConstants
import com.example.bitebound.ui.theme.BerryRed
import com.example.bitebound.ui.theme.CaramelBrown
import com.example.bitebound.ui.theme.ChocolateChip
import com.example.bitebound.ui.theme.CookieDough

/** A titled, slightly raised card used for every dashboard section. */
@Composable
fun CookieCard(
    title: String,
    emoji: String,
    modifier: Modifier = Modifier,
    content: @Composable () -> Unit,
) {
    Card(
        modifier = modifier.fillMaxWidth(),
        shape = RoundedCornerShape(24.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surface),
        elevation = CardDefaults.cardElevation(defaultElevation = 3.dp),
    ) {
        Column(Modifier.padding(18.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Text(emoji, style = MaterialTheme.typography.titleLarge)
                Spacer(Modifier.width(8.dp))
                Text(
                    title,
                    style = MaterialTheme.typography.titleLarge,
                    color = MaterialTheme.colorScheme.onSurface,
                )
            }
            Spacer(Modifier.height(14.dp))
            content()
        }
    }
}

/**
 * Score display: a plain progress ring with the count in the middle. Kept simple
 * (flat disc + track + filled part) but in warm brown cookie colors so the
 * number stays easy to read.
 */
@Composable
fun CookieProgress(
    collected: Int,
    target: Int,
    modifier: Modifier = Modifier,
) {
    val fraction = if (target > 0) (collected.toFloat() / target).coerceIn(0f, 1f) else 0f
    val animated by animateFloatAsState(
        targetValue = fraction,
        animationSpec = tween(600),
        label = "scoreProgress",
    )
    val percent = (animated * 100).toInt()

    // Brownish palette pulled from the cookie colors - a lighter chocolate disc
    // with a golden-dough fill, so the number still reads, but it's warm, not gray.
    val centerColor = ChocolateChip
    val progressColor = CookieDough
    val trackColor = CaramelBrown.copy(alpha = 0.35f)

    Box(modifier = modifier.size(210.dp), contentAlignment = Alignment.Center) {
        Canvas(Modifier.size(210.dp)) {
            val stroke = 14.dp.toPx()
            val inset = stroke / 2
            val arcSize = androidx.compose.ui.geometry.Size(size.width - stroke, size.height - stroke)

            // Flat dark circle behind the number.
            val center = Offset(size.width / 2, size.height / 2)
            val innerRadius = size.minDimension / 2 - stroke - 8.dp.toPx()
            drawCircle(color = centerColor, radius = innerRadius, center = center)

            // Empty track ring, so you can see how much is left.
            drawArc(
                color = trackColor,
                startAngle = -90f,
                sweepAngle = 360f,
                useCenter = false,
                topLeft = Offset(inset, inset),
                size = arcSize,
                style = Stroke(width = stroke, cap = StrokeCap.Round),
            )
            // Filled part grows clockwise with the score.
            drawArc(
                color = progressColor,
                startAngle = -90f,
                sweepAngle = 360f * animated,
                useCenter = false,
                topLeft = Offset(inset, inset),
                size = arcSize,
                style = Stroke(width = stroke, cap = StrokeCap.Round),
            )
        }

        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Text(
                "$collected",
                style = MaterialTheme.typography.displaySmall,
                fontWeight = FontWeight.Bold,
                color = Color.White,
            )
            Text(
                "of $target",
                style = MaterialTheme.typography.labelMedium,
                color = Color.White.copy(alpha = 0.65f),
            )
            Spacer(Modifier.height(6.dp))
            Text(
                "$percent%",
                style = MaterialTheme.typography.labelLarge,
                fontWeight = FontWeight.Bold,
                color = Color.White.copy(alpha = 0.85f),
            )
        }
    }
}

/** A compact labeled value, used in grids of sensor / physics readings. */
@Composable
fun StatTile(
    label: String,
    value: String,
    modifier: Modifier = Modifier,
    accent: Color? = null,
) {
    Card(
        modifier = modifier,
        shape = RoundedCornerShape(16.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.55f),
        ),
        elevation = CardDefaults.cardElevation(defaultElevation = 0.dp),
    ) {
        Column(
            Modifier
                .fillMaxWidth()
                .padding(vertical = 12.dp, horizontal = 12.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
        ) {
            Text(
                value,
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold,
                color = accent ?: MaterialTheme.colorScheme.onSurface,
            )
            Spacer(Modifier.height(2.dp))
            Text(
                label,
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                textAlign = TextAlign.Center,
            )
        }
    }
}

/** Lays a list of [StatTile]s out in fixed-width rows of [columns]. */
@Composable
fun StatGrid(
    stats: List<Pair<String, String>>,
    modifier: Modifier = Modifier,
    columns: Int = 3,
) {
    Column(modifier, verticalArrangement = Arrangement.spacedBy(8.dp)) {
        stats.chunked(columns).forEach { rowItems ->
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                rowItems.forEach { (label, value) ->
                    StatTile(label = label, value = value, modifier = Modifier.weight(1f))
                }
                // pad short final row so tiles keep their width
                repeat(columns - rowItems.size) {
                    Spacer(Modifier.weight(1f))
                }
            }
        }
    }
}

/**
 * Small copy of the ESP32 board that shows where the ball currently is. Keeps
 * the same width/height ratio as the real device screen so the position lines up.
 */
@Composable
fun MiniGameBoard(
    ballX: Double,
    ballY: Double,
    screenWidth: Int,
    screenHeight: Int,
    modifier: Modifier = Modifier,
    velocityX: Double = 0.0,
    velocityY: Double = 0.0,
) {
    val xFraction = if (screenWidth > 0) (ballX / screenWidth).toFloat().coerceIn(0f, 1f) else 0.5f
    val yFraction = if (screenHeight > 0) (ballY / screenHeight).toFloat().coerceIn(0f, 1f) else 0.5f
    // Red ball to match the sphere on the ESP display.
    val ballColor = BerryRed

    // Calculate aspect ratio. Default to 16:9 if unknown.
    val aspectRatio = if (screenWidth > 0 && screenHeight > 0) {
        screenWidth.toFloat() / screenHeight.toFloat()
    } else {
        16f / 9f
    }

    Box(
        modifier
            .padding(vertical = 8.dp),
        contentAlignment = Alignment.Center,
    ) {
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .aspectRatio(aspectRatio)
        ) {
            Canvas(Modifier.fillMaxSize()) {
                val pad = 8.dp.toPx()
                val boardW = size.width - pad * 2
                val boardH = size.height - pad * 2

                // Board background
                drawRoundRect(
                    color = CookieDough.copy(alpha = 0.4f),
                    topLeft = Offset(pad, pad),
                    size = androidx.compose.ui.geometry.Size(boardW, boardH),
                    cornerRadius = androidx.compose.ui.geometry.CornerRadius(16.dp.toPx()),
                )
                drawRoundRect(
                    color = CaramelBrown,
                    topLeft = Offset(pad, pad),
                    size = androidx.compose.ui.geometry.Size(boardW, boardH),
                    cornerRadius = androidx.compose.ui.geometry.CornerRadius(16.dp.toPx()),
                    style = Stroke(width = 2.dp.toPx()),
                )

                // The rolling cookie/ball
                val ballRadius = 8.dp.toPx()
                val cx = pad + ballRadius + xFraction * (boardW - ballRadius * 2)
                val cy = pad + ballRadius + yFraction * (boardH - ballRadius * 2)

                // Velocity vector
                val maxSpeed = GameConfigConstants.DEFAULT_MAX_SPEED.toFloat()
                // Scale vector length based on board size
                val vectorScale = boardW * 0.20f
                val vx = (velocityX.toFloat() / maxSpeed) * vectorScale
                val vy = (velocityY.toFloat() / maxSpeed) * vectorScale

                if (vx != 0f || vy != 0f) {
                    drawLine(
                        color = Color.Black,
                        start = Offset(cx, cy),
                        end = Offset(cx + vx, cy + vy),
                        strokeWidth = 3.dp.toPx(),
                        cap = StrokeCap.Round
                    )
                }

                drawCircle(color = ballColor, radius = ballRadius, center = Offset(cx, cy))
            }
        }
    }
}
