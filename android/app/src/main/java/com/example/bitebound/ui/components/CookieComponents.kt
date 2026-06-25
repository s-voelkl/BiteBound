package com.example.bitebound.ui.components

import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
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
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.example.bitebound.ui.theme.CaramelBrown
import com.example.bitebound.ui.theme.ChocolateChip
import com.example.bitebound.ui.theme.CookieDough
import com.example.bitebound.ui.theme.CookieGolden
import com.example.bitebound.ui.theme.Honey

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
 * The hero of the dashboard: a freshly-baked cookie with a progress ring that
 * fills as cookies are collected. [collected] / [target] drives the arc; the
 * cookie itself is hand-drawn so it scales crisply at any size.
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
        label = "cookieProgress",
    )
    val ringTrack = CookieDough.copy(alpha = 0.35f)

    Box(modifier = modifier.size(200.dp), contentAlignment = Alignment.Center) {
        Canvas(Modifier.size(200.dp)) {
            val stroke = 16.dp.toPx()
            val inset = stroke / 2
            // Progress ring
            drawArc(
                color = ringTrack,
                startAngle = -90f,
                sweepAngle = 360f,
                useCenter = false,
                topLeft = Offset(inset, inset),
                size = androidx.compose.ui.geometry.Size(size.width - stroke, size.height - stroke),
                style = Stroke(width = stroke, cap = StrokeCap.Round),
            )
            drawArc(
                brush = Brush.sweepGradient(listOf(CookieGolden, Honey, CaramelBrown, CookieGolden)),
                startAngle = -90f,
                sweepAngle = 360f * animated,
                useCenter = false,
                topLeft = Offset(inset, inset),
                size = androidx.compose.ui.geometry.Size(size.width - stroke, size.height - stroke),
                style = Stroke(width = stroke, cap = StrokeCap.Round),
            )

            // Cookie body
            val center = Offset(size.width / 2, size.height / 2)
            val cookieRadius = size.minDimension / 2 - stroke - 12.dp.toPx()
            drawCircle(
                brush = Brush.radialGradient(
                    colors = listOf(CookieDough, CookieGolden),
                    center = center,
                    radius = cookieRadius,
                ),
                radius = cookieRadius,
                center = center,
            )
            // Toasted rim
            drawCircle(
                color = CaramelBrown.copy(alpha = 0.45f),
                radius = cookieRadius,
                center = center,
                style = Stroke(width = 3.dp.toPx()),
            )
            // Chocolate chips at fixed, evenly-scattered spots
            val chips = listOf(
                Offset(-0.35f, -0.30f) to 0.13f,
                Offset(0.28f, -0.38f) to 0.10f,
                Offset(0.40f, 0.12f) to 0.12f,
                Offset(-0.05f, 0.05f) to 0.11f,
                Offset(-0.42f, 0.22f) to 0.10f,
                Offset(0.10f, 0.42f) to 0.12f,
                Offset(0.32f, 0.40f) to 0.08f,
                Offset(-0.20f, -0.05f) to 0.07f,
            )
            chips.forEach { (rel, r) ->
                drawCircle(
                    color = ChocolateChip,
                    radius = cookieRadius * r,
                    center = Offset(
                        center.x + rel.x * cookieRadius,
                        center.y + rel.y * cookieRadius,
                    ),
                )
            }
        }
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Text(
                "$collected",
                style = MaterialTheme.typography.headlineLarge,
                fontWeight = FontWeight.Bold,
                color = ChocolateChip,
            )
            Text(
                "of $target",
                style = MaterialTheme.typography.titleMedium,
                color = ChocolateChip.copy(alpha = 0.8f),
            )
        }
    }
}

/** A compact labelled value, used in grids of sensor / physics readings. */
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
    columns: Int = 3,
    modifier: Modifier = Modifier,
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
 * A miniature of the ESP32 game board showing the ball's live position. The
 * incoming coordinates are in the device's screen pixels, so they are scaled
 * into the available canvas.
 */
@Composable
fun MiniGameBoard(
    ballX: Double,
    ballY: Double,
    screenWidth: Int,
    screenHeight: Int,
    collision: Boolean,
    modifier: Modifier = Modifier,
) {
    val xFraction = if (screenWidth > 0) (ballX / screenWidth).toFloat().coerceIn(0f, 1f) else 0.5f
    val yFraction = if (screenHeight > 0) (ballY / screenHeight).toFloat().coerceIn(0f, 1f) else 0.5f
    val ballColor = if (collision) MaterialTheme.colorScheme.error else ChocolateChip

    Box(
        modifier
            .fillMaxWidth()
            .height(180.dp),
        contentAlignment = Alignment.Center,
    ) {
        Canvas(Modifier.fillMaxWidth().height(180.dp)) {
            val pad = 10.dp.toPx()
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
            val ballRadius = 9.dp.toPx()
            val cx = pad + ballRadius + xFraction * (boardW - ballRadius * 2)
            val cy = pad + ballRadius + yFraction * (boardH - ballRadius * 2)
            if (collision) {
                drawCircle(
                    color = ballColor.copy(alpha = 0.25f),
                    radius = ballRadius * 1.9f,
                    center = Offset(cx, cy),
                )
            }
            drawCircle(color = ballColor, radius = ballRadius, center = Offset(cx, cy))
            drawCircle(color = CookieGolden, radius = ballRadius * 0.4f, center = Offset(cx - 2, cy - 2))
        }
    }
}
