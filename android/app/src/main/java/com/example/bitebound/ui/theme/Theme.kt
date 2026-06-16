package com.example.bitebound.ui.theme

import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable

private val LightCookieColors = lightColorScheme(
    primary = CaramelBrown,
    onPrimary = MilkCream,
    primaryContainer = CookieDough,
    onPrimaryContainer = DarkCocoa,
    secondary = CookieGolden,
    onSecondary = DarkCocoa,
    secondaryContainer = Honey,
    onSecondaryContainer = DarkCocoa,
    tertiary = ChocolateChip,
    onTertiary = MilkCream,
    background = MilkCream,
    onBackground = DarkCocoa,
    surface = FrostingWhite,
    onSurface = DarkCocoa,
    surfaceVariant = CookieDough,
    onSurfaceVariant = Cinnamon,
    outline = CaramelBrown,
    error = BerryRed,
    onError = MilkCream,
)

private val DarkCookieColors = darkColorScheme(
    primary = NightCaramel,
    onPrimary = NightBackground,
    primaryContainer = ChocolateChip,
    onPrimaryContainer = NightChip,
    secondary = NightDough,
    onSecondary = NightBackground,
    secondaryContainer = CaramelBrown,
    onSecondaryContainer = NightChip,
    tertiary = NightChip,
    onTertiary = NightBackground,
    background = NightBackground,
    onBackground = NightCream,
    surface = NightSurface,
    onSurface = NightCream,
    surfaceVariant = ChocolateChip,
    onSurfaceVariant = NightDough,
    outline = NightCaramel,
    error = BerryRed,
    onError = MilkCream,
)

/**
 * BiteBound's cookie theme. Dynamic (wallpaper-based) colours are intentionally
 * disabled so the freshly-baked palette shows on every device.
 */
@Composable
fun BiteBoundTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    content: @Composable () -> Unit,
) {
    val colorScheme = if (darkTheme) DarkCookieColors else LightCookieColors

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content,
    )
}
